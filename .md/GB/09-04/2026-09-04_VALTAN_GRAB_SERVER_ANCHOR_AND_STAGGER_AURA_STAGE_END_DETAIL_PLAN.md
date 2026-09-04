# 2026-09-04 발탄 잡기 Server anchor 단일 경로와 마력구 오라 STAGE_END 디테일 계획서

범위와 G 순서는 [구현 계획서](2026-09-04_VALTAN_GRAB_SERVER_ANCHOR_AND_STAGGER_AURA_STAGE_END_IMPLEMENTATION_PLAN.md)가
소유한다. 이 문서는 그 G01/G02의 코드 계약만 싣는다. 설명은 반복하지 않는다.

기준: 브랜치 `GB/KoukuSaydon-Main-Pattern`, HEAD `8f8e5bea`, 2026-09-04 현재 working tree(Warp 포탈 V2 미커밋
변경 포함). 아래 모든 블록은 그 working tree 바이트를 scratchpad 사본에 실제로 적용해 만든 것이며,
저장소 자체는 이 세션에서 바꾸지 않았다. 적용 주체는 Codex/사용자다.

이 세션에서 실행한 검증:

| 항목 | 결과 |
|---|---|
| 아래 전체 변경을 담은 unified patch를 working tree에 `git apply --check` | 성공 (13 files, +40/-647) |
| 같은 patch를 `git apply --check --whitespace=error` | 성공 |
| 패치된 `BOSS_VALTAN.effectv2bindings.json` parse, bindingId 유일성 101개, STAGGER_SLOT row 3개 stopPolicy `STAGE_END, STAGE_END, NATURAL` | 성공 |
| 새 python 계약의 assert를 패치 사본 + 실제 `Character.cpp`/`GameRoom.cpp`에 대해 실행 | 성공 |
| 패치 사본의 `{`/`}`, `(`/`)` 개수가 원본과 동일 | 동일 (`Valtan.cpp`의 `(` -1은 원본에도 같은 값) |
| `validate_effect_v2.py`를 패치 전 실제 저장소에서 실행 | `131 authored, 169 bindings, 16 groups` 성공 (baseline) |

실행하지 않은 것: Debug build, `ValtanPatternAuditionServiceHarness` 실행, 패치 적용 뒤 validator/binding
pipeline/Composition Validate, runtime smoke. 이들은 적용 뒤 구현 계획서의 검증 명령으로 수행한다.

byte-exact patch 파일: `C:\Users\user\AppData\Local\Temp\claude\C--Users-user-Desktop-LostArk\d6af28a2-6f14-4bba-b6dd-ddd529def81c\scratchpad\grab-server-anchor-and-stagger-aura.patch`
(CRLF 파일의 CR을 보존한 원본이며 `git apply <path>`로 한 번에 적용할 수 있다. 부록의 diff는 읽기용으로 CR을 제거했다.)

---

## G01 잡기: Client C3 제거

### G01-1 `Client/Public/ClientReplication.h`

인코딩 UTF-8, 줄바꿈 CRLF/LF 혼재. 아래 세 블록을 그대로 삭제한다.

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: #include "ReplicatedPlayerHealth.h" 바로 아래, #include "CombatDebugVisibility.h" 바로 위
```

```cpp
#include "PlayerHandGripTransform.h"
```

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: void Release_CombatObjectPresentation(COMBAT_OBJECT_PRESENTATION_HANDLE handle); 바로 아래,
      struct COMBAT_OBJECT_PRESENTATION_SINK final 바로 위
```

```cpp
		void Stage_PlayerAttachmentPresentation(
			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
		void Update_PlayerAttachmentPresentations();
```

```text
파일: Client/Public/ClientReplication.h
작업: 삭제
기준점: std::string m_strPendingPresentationFailure; 바로 아래,
      VALTAN_PRESENTATION_STATE m_ValtanPresentationState; 바로 위
```

```cpp
		struct PLAYER_ATTACHMENT_PRESENTATION final
		{
			LostArk::Shared::NET_ENTITY_ID iOwnerNetEntityId =
				LostArk::Shared::INVALID_NET_ENTITY_ID;
			LostArk::Shared::PLAYER_ATTACHMENT_SLOT eSlot =
				LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
			float4x4_t LocalOffset{};
			bool_t bHasLocalOffset = false;
			PLAYER_HAND_GRIP_LOCAL_OFFSET GripLocalOffset{};
			bool_t bHasGripLocalOffset = false;
		};
		std::unordered_map<
			LostArk::Shared::NET_ENTITY_ID,
			PLAYER_ATTACHMENT_PRESENTATION> m_PlayerAttachments;
```

### G01-2 `Client/Private/ClientReplication.cpp`

인코딩 UTF-8, 줄바꿈 CRLF/LF 혼재. 여섯 곳을 삭제한다. 추가되는 코드는 없다.

```text
작업 1: 삭제 (익명 namespace, Is_FiniteMatrix 정의 바로 위. 상수 한 줄과 그 아래 빈 줄 하나)
```

```cpp
	constexpr const char* VALTAN_LEFT_HAND_BONE = "bip001-l-hand";

```

```text
작업 2: 삭제 (CClientReplication::Update 끝부분, Update_DeathPresentations(); 바로 아래 한 줄)
```

```cpp
	Update_PlayerAttachmentPresentations();
```

```text
작업 3: 삭제 (despawn 처리, if (!m_Registry.Unregister(despawned.iNetEntityId)) return false; 바로 아래 한 줄)
```

```cpp
	m_PlayerAttachments.erase(despawned.iNetEntityId);
```

```text
작업 4: 삭제 (함수 정의 2개 전체.
      시작: void Client::CClientReplication::Stage_PlayerAttachmentPresentation( 줄
      끝: bool Client::CClientReplication::Apply_WorldSnapshot( 줄 바로 전까지.
      Stop_CombatObjectPresentation 정의의 닫는 } 와 그 뒤 빈 줄 하나는 남긴다)
      삭제되는 본문 전문은 부록 diff의 ClientReplication.cpp 세 번째 hunk와 같다
```

```text
작업 5: 삭제 (Apply_WorldSnapshot player loop, character->Apply_NetworkStance(player.eStance); 바로 위 한 줄)
```

```cpp
		Stage_PlayerAttachmentPresentation(player);
```

```text
작업 6: 삭제 (reset 경로, m_Registry.Reset(); 바로 아래 한 줄)
```

```cpp
	m_PlayerAttachments.clear();
```

### G01-3 `Client/Public/Valtan.h`

인코딩 ASCII, 줄바꿈 CRLF/LF 혼재. 네 블록을 삭제한다.

```text
작업 1: 삭제 (#include "ValtanPresentationGenerationAdmission.h" 바로 아래 한 줄)
```

```cpp
#include "PlayerHandGripTransform.h"
```

```text
작업 2: 삭제 (const std::string& Get_ServerActionId() const 바로 아래, #ifdef _DEBUG 바로 위)
```

```cpp
	bool_t Try_Get_PlayerHandGripLocalOffset(
		std::string_view actionId,
		Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
	bool_t Try_Get_PlayerHandGripLocalOffsetByPatternId(
		std::string_view patternId,
		Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
```

```text
작업 3: 삭제 (m_PatternBodyVisibilityByActionId; 바로 아래, bool_t m_bLocalPatternAuthoringPreview = false; 바로 위)
```

```cpp
	/* Product capture presentation retains action bindings for authoring
	   diagnostics. Runtime attachment lookup uses the replicated patternId,
	   which survives a same-tick branch away from the CAPTURE stage. */
	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
		m_PlayerHandGripLocalOffsetByActionId;
	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
		m_PlayerHandGripLocalOffsetByPatternId;
```

```text
작업 4: 삭제 (bool_t Reload_PatternBindings_WhileAdmitted(std::string& strOutStatus); 바로 아래)
```

```cpp
	bool_t Reload_PlayerHandGripLocalOffsets_WhileAdmitted(
		std::string& strOutStatus);
```

### G01-4 `Client/Private/Valtan.cpp`

인코딩 ASCII, 줄바꿈 CRLF/LF 혼재. 함수 정의 3개와 joined reload의 grip 단계 8곳을 삭제하고 문자열 하나를 교체한다.

```text
작업 1: 삭제 (함수 정의 3개 전체.
      시작: bool_t CValtan::Reload_PlayerHandGripLocalOffsets_WhileAdmitted( 줄
      끝: 첫 번째 overload  bool_t CValtan::Reload_PatternPresentationAuthoring(\n\tstd::string& strOutStatus)\n{  바로 전까지.
      두 번째 overload(GameplayDataRevision 인자)는 대상이 아니다.
      삭제 본문 전문은 부록 diff의 Valtan.cpp 첫 hunk와 같다)
```

이하는 모두 `Reload_PatternPresentationAuthoring_Impl` 안이다.

```text
작업 2: 삭제 (const auto PreviousBodyVisibility = m_PatternBodyVisibilityByActionId; 바로 아래 4줄)
```

```cpp
	const auto PreviousGripLocalOffsetsByActionId =
		m_PlayerHandGripLocalOffsetByActionId;
	const auto PreviousGripLocalOffsetsByPatternId =
		m_PlayerHandGripLocalOffsetByPatternId;
```

```text
작업 3: 삭제 (RestorePrevious lambda capture 목록, &PreviousBindings, &PreviousBodyVisibility, 바로 아래 2줄)
```

```cpp
		&PreviousGripLocalOffsetsByActionId,
		&PreviousGripLocalOffsetsByPatternId,
```

```text
작업 4: 삭제 (lambda 본문, m_PatternBodyVisibilityByActionId = PreviousBodyVisibility; 바로 아래 4줄)
```

```cpp
		m_PlayerHandGripLocalOffsetByActionId =
			PreviousGripLocalOffsetsByActionId;
		m_PlayerHandGripLocalOffsetByPatternId =
			PreviousGripLocalOffsetsByPatternId;
```

```text
작업 5: 삭제 (if (!Reload_PatternBindings_WhileAdmitted(StepStatus) || 바로 아래 한 줄)
```

```cpp
		!Reload_PlayerHandGripLocalOffsets_WhileAdmitted(StepStatus) ||
```

```text
작업 6: 교체 (같은 if 블록의 실패 상태 문자열 한 곳)
```

```cpp
			"Valtan joined presentation reload rejected; every previous animation/grip/effect/sound/combat-sound/shake cache was preserved: " +
```

→

```cpp
			"Valtan joined presentation reload rejected; every previous animation/effect/sound/combat-sound/shake cache was preserved: " +
```

```text
작업 7: 삭제 (staged commit 준비, auto StagedBodyVisibility = std::move(m_PatternBodyVisibilityByActionId); 바로 아래 4줄)
```

```cpp
	auto StagedGripLocalOffsetsByActionId =
		std::move(m_PlayerHandGripLocalOffsetByActionId);
	auto StagedGripLocalOffsetsByPatternId =
		std::move(m_PlayerHandGripLocalOffsetByPatternId);
```

```text
작업 8: 삭제 (aggregate commit, m_PatternBodyVisibilityByActionId = std::move(StagedBodyVisibility); 바로 아래 4줄)
```

```cpp
	m_PlayerHandGripLocalOffsetByActionId =
		std::move(StagedGripLocalOffsetsByActionId);
	m_PlayerHandGripLocalOffsetByPatternId =
		std::move(StagedGripLocalOffsetsByPatternId);
```

### G01-5 `Client/Public/PlayerHandGripTransform.h`

인코딩 ASCII, 줄바꿈 CRLF. 파일 전체를 아래 전문으로 교체한다. `Client.vcxproj`/`.filters` 등록은 유지한다.

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

### G01-6 harness

```text
작업 1: 파일 삭제
파일: Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp
```

```text
작업 2: 삭제
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
기준점: <ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp" /> 바로 아래 한 줄
유지: <ClInclude Include="..\..\..\Client\Public\PlayerHandGripTransform.h" />
```

```xml
	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
```

```text
작업 3: 삭제
파일: Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
기준점: <ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp" /> 바로 아래 한 줄
유지: ClInclude 항목
```

```xml
	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
```

```text
작업 4: 삭제 3곳
파일: Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp (ASCII, CRLF)
```

```cpp
int Run_PlayerHandGripTransformContractTests();
```

```cpp
	const int PlayerHandGripTransformFailures =
		Run_PlayerHandGripTransformContractTests();
```

```cpp
		0 == PlayerHandGripTransformFailures &&
```

### G01-7 `Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py`

줄바꿈 LF. 적용 뒤 파일 전문이다. 상수 두 줄이 늘고 세 번째 테스트 메서드가 교체된다.

```python
import copy
import json
import math
import unittest
from pathlib import Path

from Tools.ValtanPipeline import valtan_tuning_pipeline as pipeline


ROOT = Path(__file__).resolve().parents[2]
GAMEPLAY = ROOT / "Data/Valtan/Valtan.gameplay.json"
VALTAN_HEADER = ROOT / "Client/Public/Valtan.h"
VALTAN_SOURCE = ROOT / "Client/Private/Valtan.cpp"
REPLICATION_SOURCE = ROOT / "Client/Private/ClientReplication.cpp"
CHARACTER_SOURCE = ROOT / "Client/Private/Character.cpp"
GAME_ROOM_SOURCE = ROOT / "Server/Private/GameRoom.cpp"


def load_gameplay() -> dict:
    return json.loads(GAMEPLAY.read_text(encoding="utf-8"))


def capture_hits(document: dict) -> list[dict]:
    return [
        stage["hit"]
        for pattern in document["patterns"]
        for stage in pattern["stages"]
        if stage["hit"].get("playerResponse") == "CAPTURE"
    ]


class ValtanGripLocalOffsetContractTests(unittest.TestCase):
    def test_all_stable_capture_actions_project_the_authored_grip(self) -> None:
        document = load_gameplay()
        pipeline.validate_gameplay_authoring(document)
        hits = capture_hits(document)
        self.assertEqual(7, len(hits))
        expected = {"forwardM": 0.0, "upM": -0.9, "rightM": 0.0}
        for hit in hits:
            self.assertEqual(expected, hit["gripLocalOffset"])
            projected = pipeline._compile_hit(hit)
            self.assertEqual(expected, projected["gripLocalOffset"])
            self.assertEqual("CAPTURE", projected["playerResponse"])
            self.assertEqual("BOSS_LEFT_HAND", projected["attachmentSlot"])

    def test_capture_grip_is_deterministic_per_pattern(self) -> None:
        bindings = {}
        for pattern in load_gameplay()["patterns"]:
            grips = [
                stage["hit"]["gripLocalOffset"]
                for stage in pattern["stages"]
                if stage["hit"].get("playerResponse") == "CAPTURE"
            ]
            if not grips:
                continue
            self.assertTrue(all(grip == grips[0] for grip in grips))
            bindings[pattern["patternId"]] = grips[0]
        self.assertEqual(
            {"VALTAN_TRASH", "VALTAN_TRASH_CATCH_IF", "VALTAN_CATCH_BREATH"},
            set(bindings),
        )

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

    def test_missing_grip_rejects_without_mutating_committed_source(self) -> None:
        committed = load_gameplay()
        baseline = copy.deepcopy(committed)
        candidate = copy.deepcopy(committed)
        del capture_hits(candidate)[0]["gripLocalOffset"]
        with self.assertRaisesRegex(
            pipeline.PipelineError,
            "requires playerResponse, attachmentSlot, and gripLocalOffset together",
        ):
            pipeline.validate_gameplay_authoring(candidate)
        self.assertEqual(baseline, committed)

    def test_nonfinite_out_of_range_and_hidden_fields_are_rejected(self) -> None:
        for field, value in (
            ("forwardM", math.nan),
            ("upM", math.inf),
            ("rightM", 10.01),
        ):
            candidate = load_gameplay()
            capture_hits(candidate)[0]["gripLocalOffset"][field] = value
            with self.assertRaises(pipeline.PipelineError):
                pipeline.validate_gameplay_authoring(candidate)

        candidate = load_gameplay()
        capture_hits(candidate)[0]["gripLocalOffset"]["hidden"] = 1.0
        with self.assertRaisesRegex(pipeline.PipelineError, "fields mismatch"):
            pipeline.validate_gameplay_authoring(candidate)


if __name__ == "__main__":
    unittest.main()
```

### G01-8 팀 문서

```text
파일: .md/TEAM/발탄인수인계서.md (UTF-8, CRLF)
작업: 교체 (연속된 두 줄)
```

기존:

```text
Client grab은 왼손 본의 고정 local origin에 붙인다. capture 시점의 상대 위치는 보존하지 않고 회전과
player scale만 보존한다. 실제 0.01 hand scale도 허용하도록 정규화한 basis determinant를 검사한다.
```

교체:

```text
Client grab은 손 bone 합성 없이 Server attachment anchor를 그대로 쓴다. Server는 capture 순간의
boss-local 상대 위치와 yaw 오프셋을 저장해 매 tick `boss pos + yaw 회전(offset)`을 플레이어 위치로
복제하고, Client `CCharacter::Update_NetworkTransform`이 그 값을 보간해 transform에 쓴다. 본체,
nameplate, Debug wire, Server 판정이 같은 좌표를 사용한다. `gripLocalOffset`은 저작 데이터로 남지만
runtime 소비자는 없다.
```

```text
파일: .md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md (UTF-8, CRLF)
작업: 교체 (연속된 세 줄, "필드를 반드시 invalid로 둔다. " 접두는 유지)
```

기존:

```text
필드를 반드시 invalid로 둔다. Client는 대상을 다시 고르지 않으며, 62줄 잡기 표현은 이 ID로
찾은 Character만 `bip001-l-hand` animated socket에 붙였다가 success/recovery 또는 패턴 종료 시
Server snapshot transform으로 돌려놓는다.
```

교체:

```text
필드를 반드시 invalid로 둔다. Client는 대상을 다시 고르지 않는다. 잡힌 Character의 transform은
`PLAYER_SNAPSHOT`의 GRABBED 상태와 Server attachment anchor 위치를
`CCharacter::Update_NetworkTransform`이 보간한 값이며, Client가 boss 손 bone에 다시 붙이지 않는다.
release 뒤에도 같은 snapshot 경로가 이어진다.
```

---

## G02 마력구 오라 binding과 STAGE_END

### G02-1 `Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json`

줄바꿈 LF. 변경은 두 곳이다.

```text
작업 1: 교체
기준점: "bindingId": "binding.valtan.migrated.017.1f4a28d150fc1e51" row 안의 마지막 필드
```

```json
      "stopPolicy": "NATURAL"
```

→

```json
      "stopPolicy": "STAGE_END"
```

```text
작업 2: 추가 (위 row의 닫는  "    },"  바로 아래 한 줄. 기존 project-tuned row와 같은 한 줄 형식)
```

```json
    { "bindingId": "binding.valtan.project-tuned.stagger-slot.channel.aura", "resource": { "kind": "GROUP", "id": "boss.valtan.magicball.aura" }, "scope": { "patternId": "VALTAN_STAGGER_SLOT", "stageId": "CHANNEL", "actionId": "valtan.authoring.stagger-slot.channel" }, "clock": { "basis": "STAGE", "clipOccurrenceId": null, "startMs": 0, "repeatPolicy": "ONCE" }, "anchor": { "slotId": "b_effectroot", "followPolicy": "FOLLOW_SLOT", "rotationBasis": "TARGET_YAW", "localTransform": { "translation": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0] } }, "stopPolicy": "STAGE_END" },
```

적용 뒤 `VALTAN_STAGGER_SLOT` row는 순서대로 magicball(`STAGE_END`), magicball.aura(`STAGE_END`),
final-attack wipe(`NATURAL`) 세 개다. runtime, Server, Composition manifest 변경은 없다.

---

## 적용 뒤 검증 명령

구현 계획서 G01/G02 검증 절과 같다. 요약:

```powershell
PYTHONPATH=. PYTHONIOENCODING=utf-8 python -m unittest Tools.ValtanPipeline.test_valtan_grip_local_offset_contract
```

```powershell
python -B Tools/EffectToolV2/validate_effect_v2.py --repository-root . --resource-root Client/Bin/Resources
```

```powershell
python -B Tools/EffectToolV2/effect_v2_binding_pipeline.py --repository-root . validate --bindings Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
```

```powershell
PYTHONPATH=. PYTHONIOENCODING=utf-8 python -m unittest Tools.EffectToolV2.test_effect_v2_binding_pipeline Tools.EffectToolV2.test_valtan_magicball_black_core_contract Tools.EffectToolV2.test_validate_effect_v2 Tools.EffectToolV2.test_effect_v2_occurrence_runtime_contract Tools.EffectToolV2.test_effect_v2_product_contract
```

```powershell
powershell -ExecutionPolicy Bypass -File Tools/CompositionPipeline/Publish-Compositions.ps1 -Mode Validate -RepositoryRoot .
```

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile FullDiagnostic
```

```powershell
git diff --check
```

수동 확인(사용자): TRASH 잡기, CATCH_BREATH 잡기에서 본체·nameplate·Debug wire 일치. STAGGER_SLOT에서 오라와
마력구 동시 재생, 1000 damage groggy 진입 프레임과 12초 TIMEOUT FINAL_ATTACK 진입 프레임에 둘 다 소멸.

---

## 부록. 검증된 전체 diff (읽기용, CR 제거)

`git apply --check --whitespace=error` 통과본이다. 적용은 위 byte-exact patch 파일로 한다.

````diff
--- a/Client/Public/ClientReplication.h
+++ b/Client/Public/ClientReplication.h
@@ -11,7 +11,6 @@
 #include "WorldDestructionProjectionDocument.h"
 #include "WorldDestructionProjectionRuntime.h"
 #include "ReplicatedPlayerHealth.h"
-#include "PlayerHandGripTransform.h"
 #include "CombatDebugVisibility.h"
 
 #include <chrono>
@@ -511,9 +510,6 @@
 			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
 		void Release_CombatObjectPresentation(
 			COMBAT_OBJECT_PRESENTATION_HANDLE handle);
-		void Stage_PlayerAttachmentPresentation(
-			const LostArk::Shared::PLAYER_SNAPSHOT& snapshot);
-		void Update_PlayerAttachmentPresentations();
 
 		struct COMBAT_OBJECT_PRESENTATION_SINK final
 		{
@@ -566,20 +562,6 @@
 		} m_DeferredLocalCharacterClassReplacement;
 		std::uint64_t m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
 		std::string m_strPendingPresentationFailure;
-		struct PLAYER_ATTACHMENT_PRESENTATION final
-		{
-			LostArk::Shared::NET_ENTITY_ID iOwnerNetEntityId =
-				LostArk::Shared::INVALID_NET_ENTITY_ID;
-			LostArk::Shared::PLAYER_ATTACHMENT_SLOT eSlot =
-				LostArk::Shared::PLAYER_ATTACHMENT_SLOT::NONE;
-			float4x4_t LocalOffset{};
-			bool_t bHasLocalOffset = false;
-			PLAYER_HAND_GRIP_LOCAL_OFFSET GripLocalOffset{};
-			bool_t bHasGripLocalOffset = false;
-		};
-		std::unordered_map<
-			LostArk::Shared::NET_ENTITY_ID,
-			PLAYER_ATTACHMENT_PRESENTATION> m_PlayerAttachments;
 		VALTAN_PRESENTATION_STATE m_ValtanPresentationState;
 		CCombatObjectProjectionRuntime m_CombatObjectProjectionRuntime;
 		CWorldDestructionProjectionRuntime m_WorldDestructionProjectionRuntime;
--- a/Client/Private/ClientReplication.cpp
+++ b/Client/Private/ClientReplication.cpp
@@ -112,8 +112,6 @@
 			left.fYawDegrees == right.fYawDegrees;
 	}
 
-	constexpr const char* VALTAN_LEFT_HAND_BONE = "bip001-l-hand";
-
 	bool Is_FiniteMatrix(const float4x4_t& matrix)
 	{
 		for (std::size_t row = 0u; row < 4u; ++row)
@@ -369,7 +367,6 @@
 	}
 
 	Update_DeathPresentations();
-	Update_PlayerAttachmentPresentations();
 #ifdef _DEBUG
 	if (m_CombatDebugVisibility.bCombatObjectHit)
 		Draw_CombatObjectHitAreaDebug();
@@ -1569,7 +1566,6 @@
 
 	if (!m_Registry.Unregister(despawned.iNetEntityId))
 		return false;
-	m_PlayerAttachments.erase(despawned.iNetEntityId);
 
 	if (m_LocalCharacterHandle.iSlotIndex == handle.iSlotIndex &&
 		m_LocalCharacterHandle.iGeneration == handle.iGeneration)
@@ -2419,140 +2415,6 @@
 		CEffectV2Runtime::Stop_Group(static_cast<uint32_t>(handle.iValue));
 }
 
-void Client::CClientReplication::Stage_PlayerAttachmentPresentation(
-	const LostArk::Shared::PLAYER_SNAPSHOT& snapshot)
-{
-	using namespace LostArk::Shared;
-	if (PLAYER_ACTION_STATE::GRABBED != snapshot.eAction)
-	{
-		m_PlayerAttachments.erase(snapshot.iNetEntityId);
-		return;
-	}
-
-	PLAYER_ATTACHMENT_PRESENTATION& presentation =
-		m_PlayerAttachments[snapshot.iNetEntityId];
-	if (presentation.iOwnerNetEntityId !=
-			snapshot.iAttachmentOwnerNetEntityId ||
-		presentation.eSlot != snapshot.eAttachmentSlot)
-	{
-		presentation = {};
-		presentation.iOwnerNetEntityId =
-			snapshot.iAttachmentOwnerNetEntityId;
-		presentation.eSlot = snapshot.eAttachmentSlot;
-		/* The replicated offset is expressed in the boss gameplay-root frame and
-		cannot be composed with a presentation bone. Capture the hand-local matrix
-		once both actual presentation transforms are available below. */
-		presentation.bHasLocalOffset = false;
-		presentation.bHasGripLocalOffset = false;
-	}
-}
-
-void Client::CClientReplication::Update_PlayerAttachmentPresentations()
-{
-	using namespace LostArk::Shared;
-	for (auto attachment = m_PlayerAttachments.begin();
-		attachment != m_PlayerAttachments.end();)
-	{
-		OBJECT_HANDLE playerHandle{};
-		if (!m_Registry.Find_Handle(attachment->first, playerHandle))
-		{
-			attachment = m_PlayerAttachments.erase(attachment);
-			continue;
-		}
-		const std::shared_ptr<CCharacter> character =
-			m_Registry.Resolve(playerHandle);
-		const auto owner =
-			m_WorldEntities.find(attachment->second.iOwnerNetEntityId);
-		if (nullptr == character || nullptr == character->Get_Transform() ||
-			m_WorldEntities.end() == owner ||
-			WORLD_ENTITY_KIND::BOSS != owner->second.eKind ||
-			owner->second.bPresentationIsolated ||
-			PLAYER_ATTACHMENT_SLOT::BOSS_LEFT_HAND !=
-				attachment->second.eSlot)
-		{
-			/* Apply_NetworkState already staged the Server fallback transform.
-			Keep the identity so a presentation that spawns later can attach, but
-			never apply a stale matrix while its owner is unavailable. */
-			++attachment;
-			continue;
-		}
-
-		const std::shared_ptr<CValtan> valtan = owner->second.pValtan.lock();
-		const std::shared_ptr<Engine::CModel> body =
-			nullptr == valtan ? nullptr : valtan->Get_BodyModel();
-		float4x4_t presentationRoot{};
-		if (nullptr == valtan || nullptr == body ||
-			!body->Has_Bone(VALTAN_LEFT_HAND_BONE) ||
-			!valtan->Try_Get_PresentationRootMatrix(&presentationRoot) ||
-			!Is_FiniteMatrix(presentationRoot))
-		{
-			++attachment;
-			continue;
-		}
-
-		const matrix_t handWorld = body->Get_BoneMatrix(
-			VALTAN_LEFT_HAND_BONE) * XMLoadFloat4x4(&presentationRoot);
-		float4x4_t handWorldStored{};
-		XMStoreFloat4x4(&handWorldStored, handWorld);
-		if (!Is_FiniteMatrix(handWorldStored))
-		{
-			++attachment;
-			continue;
-		}
-		if (!attachment->second.bHasLocalOffset)
-		{
-			float4x4_t localOffset{};
-			if (!CPlayerHandGripTransform::Build_LocalOffset(
-					*character->Get_Transform()->Get_WorldMatrixPtr(),
-					handWorldStored, localOffset))
-			{
-				++attachment;
-				continue;
-			}
-			attachment->second.GripLocalOffset = {};
-			attachment->second.bHasGripLocalOffset =
-				valtan->Try_Get_PlayerHandGripLocalOffsetByPatternId(
-				valtan->Get_ServerPatternId(),
-				attachment->second.GripLocalOffset);
-			if (!attachment->second.bHasGripLocalOffset)
-			{
-				m_strPendingPresentationFailure =
-					"Valtan left-hand attachment retained its Server fallback because the replicated Pattern has no admitted gripLocalOffset: " +
-					(valtan->Get_ServerPatternId().empty() ?
-						std::string("<empty>") : valtan->Get_ServerPatternId());
-				++attachment;
-				continue;
-			}
-			attachment->second.LocalOffset = localOffset;
-			attachment->second.bHasLocalOffset = true;
-		}
-
-		/* The authored correction moves the character's feet-origin to the palm
-		   in normalized wrist-local axes. The captured orientation/scale still
-		   follows the H3 local * hand contract, never the hit distance. */
-		float4x4_t attachedStored{};
-		if (!CPlayerHandGripTransform::Compose_World(
-				attachment->second.LocalOffset, handWorldStored,
-				attachment->second.bHasGripLocalOffset ?
-					attachment->second.GripLocalOffset :
-					PLAYER_HAND_GRIP_LOCAL_OFFSET{}, attachedStored))
-		{
-			++attachment;
-			continue;
-		}
-		const matrix_t attachedWorld = XMLoadFloat4x4(&attachedStored);
-
-		const std::shared_ptr<Engine::CTransform> transform =
-			character->Get_Transform();
-		transform->Set_State(STATE::RIGHT, attachedWorld.r[0]);
-		transform->Set_State(STATE::UP, attachedWorld.r[1]);
-		transform->Set_State(STATE::LOOK, attachedWorld.r[2]);
-		transform->Set_State(
-			STATE::POSITION, XMVectorSetW(attachedWorld.r[3], 1.f));
-		++attachment;
-	}
-}
-
 bool Client::CClientReplication::Apply_WorldSnapshot(
 	const LostArk::Shared::S2C_WORLD_SNAPSHOT& snapshot)
 {
@@ -2660,7 +2522,6 @@
 		{
 			allSucceeded = false;
 		}
-		Stage_PlayerAttachmentPresentation(player);
 		character->Apply_NetworkStance(player.eStance);
 		if (isLocallyControlled)
 		{
@@ -3247,7 +3108,6 @@
 	}
 	m_DeathPresentations.clear();
 	m_Registry.Reset();
-	m_PlayerAttachments.clear();
 	m_LocalCharacterHandle = {};
 	Clear_DeferredLocalCharacterClassReplacement();
 	m_iNextDeferredLocalCharacterClassReplacementGeneration = 1u;
--- a/Client/Public/Valtan.h
+++ b/Client/Public/Valtan.h
@@ -12,7 +12,6 @@
 #include "ValtanPatternSoundCueDocument.h"
 #include "ValtanCombatObjectSoundCueDocument.h"
 #include "ValtanPresentationGenerationAdmission.h"
-#include "PlayerHandGripTransform.h"
 
 #include <algorithm>
 #include <cmath>
@@ -345,12 +344,6 @@
 	}
 	const std::string& Get_ServerPatternId() const { return m_strServerPatternId; }
 	const std::string& Get_ServerActionId() const { return m_strServerActionId; }
-	bool_t Try_Get_PlayerHandGripLocalOffset(
-		std::string_view actionId,
-		Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
-	bool_t Try_Get_PlayerHandGripLocalOffsetByPatternId(
-		std::string_view patternId,
-		Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const;
 #ifdef _DEBUG
 	/* Process-local visual A/B only.  Server pattern timing and the Product V0
 	   cue document remain authoritative in both modes. */
@@ -502,13 +495,6 @@
 	};
 	std::unordered_map<std::string, PATTERN_BODY_VISIBILITY_WINDOW>
 		m_PatternBodyVisibilityByActionId;
-	/* Product capture presentation retains action bindings for authoring
-	   diagnostics. Runtime attachment lookup uses the replicated patternId,
-	   which survives a same-tick branch away from the CAPTURE stage. */
-	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
-		m_PlayerHandGripLocalOffsetByActionId;
-	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
-		m_PlayerHandGripLocalOffsetByPatternId;
 	bool_t m_bLocalPatternAuthoringPreview = false;
 	std::unordered_map<std::string,
 		std::vector<BOSS_PATTERN_ANIMATION_CLIP>>
@@ -670,8 +656,6 @@
 	HRESULT Ready_Components(f32_t collisionRadius);
 	void Load_PatternBindings();
 	bool_t Reload_PatternBindings_WhileAdmitted(std::string& strOutStatus);
-	bool_t Reload_PlayerHandGripLocalOffsets_WhileAdmitted(
-		std::string& strOutStatus);
 	bool_t Apply_PatternPresentationSample(
 		std::string_view actionId,
 		std::string_view fallbackClipName,
--- a/Client/Private/Valtan.cpp
+++ b/Client/Private/Valtan.cpp
@@ -762,110 +762,6 @@
 	return true;
 }
 
-bool_t CValtan::Reload_PlayerHandGripLocalOffsets_WhileAdmitted(
-	std::string& strOutStatus)
-{
-	CEncounterPatternReference encounter;
-	if (!encounter.Load(CProjectDataRoot::Resolve(
-			std::filesystem::path(L"Encounters") / L"Valtan" /
-			L"ValtanEncounter.json"), strOutStatus))
-	{
-		strOutStatus =
-			"Valtan player hand-grip Product admission rejected: " +
-			strOutStatus;
-		return false;
-	}
-
-	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
-		stagedByActionId;
-	std::unordered_map<std::string, PLAYER_HAND_GRIP_LOCAL_OFFSET>
-		stagedByPatternId;
-	for (const ENCOUNTER_PATTERN_REFERENCE& pattern : encounter.Get_Patterns())
-	{
-		bool_t bHasPatternGripLocalOffset = false;
-		PLAYER_HAND_GRIP_LOCAL_OFFSET patternGripLocalOffset{};
-		for (const ENCOUNTER_STAGE_REFERENCE& stage : pattern.stages)
-		{
-			if (!stage.gripLocalOffset.has_value())
-				continue;
-			if (pattern.patternId.empty() || stage.actionId.empty() ||
-				!CPlayerHandGripTransform::Is_ValidGripLocalOffset(
-					*stage.gripLocalOffset) ||
-				!stagedByActionId.emplace(
-					stage.actionId, *stage.gripLocalOffset).second)
-			{
-				strOutStatus =
-					"Valtan player hand-grip stable action identity is invalid: " +
-					pattern.patternId + "/" + stage.stageId;
-				return false;
-			}
-			if (!bHasPatternGripLocalOffset)
-			{
-				patternGripLocalOffset = *stage.gripLocalOffset;
-				bHasPatternGripLocalOffset = true;
-			}
-			else if (patternGripLocalOffset != *stage.gripLocalOffset)
-			{
-				strOutStatus =
-					"Valtan player hand-grip Pattern has conflicting CAPTURE gripLocalOffset values: " +
-					pattern.patternId;
-				return false;
-			}
-		}
-		if (bHasPatternGripLocalOffset &&
-			!stagedByPatternId.emplace(
-				pattern.patternId, patternGripLocalOffset).second)
-		{
-			strOutStatus =
-				"Valtan player hand-grip stable Pattern identity is duplicated: " +
-				pattern.patternId;
-			return false;
-		}
-	}
-	m_PlayerHandGripLocalOffsetByActionId = std::move(stagedByActionId);
-	m_PlayerHandGripLocalOffsetByPatternId = std::move(stagedByPatternId);
-	strOutStatus = "Reloaded " +
-		std::to_string(m_PlayerHandGripLocalOffsetByActionId.size()) +
-		" Valtan player hand-grip action binding(s) and " +
-		std::to_string(m_PlayerHandGripLocalOffsetByPatternId.size()) +
-		" Pattern binding(s).";
-	return true;
-}
-
-bool_t CValtan::Try_Get_PlayerHandGripLocalOffset(
-	const std::string_view actionId,
-	Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
-{
-	if (actionId.empty())
-		return false;
-	const auto found = m_PlayerHandGripLocalOffsetByActionId.find(
-		std::string(actionId));
-	if (m_PlayerHandGripLocalOffsetByActionId.end() == found ||
-		!CPlayerHandGripTransform::Is_ValidGripLocalOffset(found->second))
-	{
-		return false;
-	}
-	outOffset = found->second;
-	return true;
-}
-
-bool_t CValtan::Try_Get_PlayerHandGripLocalOffsetByPatternId(
-	const std::string_view patternId,
-	Client::PLAYER_HAND_GRIP_LOCAL_OFFSET& outOffset) const
-{
-	if (patternId.empty())
-		return false;
-	const auto found = m_PlayerHandGripLocalOffsetByPatternId.find(
-		std::string(patternId));
-	if (m_PlayerHandGripLocalOffsetByPatternId.end() == found ||
-		!CPlayerHandGripTransform::Is_ValidGripLocalOffset(found->second))
-	{
-		return false;
-	}
-	outOffset = found->second;
-	return true;
-}
-
 bool_t CValtan::Reload_PatternPresentationAuthoring(
 	std::string& strOutStatus)
 {
@@ -931,10 +827,6 @@
 	const auto PreviousBindings = m_PatternClipByActionId;
 	const auto PreviousBodyVisibility =
 		m_PatternBodyVisibilityByActionId;
-	const auto PreviousGripLocalOffsetsByActionId =
-		m_PlayerHandGripLocalOffsetByActionId;
-	const auto PreviousGripLocalOffsetsByPatternId =
-		m_PlayerHandGripLocalOffsetByPatternId;
 	const auto PreviousEffectCues = m_PatternEffectCuesByActionId;
 	const auto PreviousArenaCenters = m_PatternArenaCenterAnchors;
 	const auto PreviousEffectAttempts = m_AttemptedPatternEffectOccurrenceKeys;
@@ -958,8 +850,6 @@
 
 	const auto RestorePrevious = [this,
 		&PreviousBindings, &PreviousBodyVisibility,
-		&PreviousGripLocalOffsetsByActionId,
-		&PreviousGripLocalOffsetsByPatternId,
 		&PreviousEffectCues, &PreviousArenaCenters,
 		&PreviousEffectAttempts, PreviousEffectScanValid,
 		PreviousEffectScanAge, &PreviousSoundCues,
@@ -972,10 +862,6 @@
 	{
 		m_PatternClipByActionId = PreviousBindings;
 		m_PatternBodyVisibilityByActionId = PreviousBodyVisibility;
-		m_PlayerHandGripLocalOffsetByActionId =
-			PreviousGripLocalOffsetsByActionId;
-		m_PlayerHandGripLocalOffsetByPatternId =
-			PreviousGripLocalOffsetsByPatternId;
 		m_PatternEffectCuesByActionId = PreviousEffectCues;
 		m_PatternArenaCenterAnchors = PreviousArenaCenters;
 		m_AttemptedPatternEffectOccurrenceKeys = PreviousEffectAttempts;
@@ -998,7 +884,6 @@
 
 	std::string StepStatus;
 	if (!Reload_PatternBindings_WhileAdmitted(StepStatus) ||
-		!Reload_PlayerHandGripLocalOffsets_WhileAdmitted(StepStatus) ||
 		!Reload_PatternEffectCues_WhileAdmitted(StepStatus) ||
 		!Reload_PatternSoundCues_WhileAdmitted(StepStatus) ||
 		!Reload_CombatObjectSoundCues_WhileAdmitted(StepStatus) ||
@@ -1006,7 +891,7 @@
 	{
 		RestorePrevious();
 		strOutStatus =
-			"Valtan joined presentation reload rejected; every previous animation/grip/effect/sound/combat-sound/shake cache was preserved: " +
+			"Valtan joined presentation reload rejected; every previous animation/effect/sound/combat-sound/shake cache was preserved: " +
 			StepStatus;
 		return false;
 	}
@@ -1019,10 +904,6 @@
 	auto StagedBindings = std::move(m_PatternClipByActionId);
 	auto StagedBodyVisibility =
 		std::move(m_PatternBodyVisibilityByActionId);
-	auto StagedGripLocalOffsetsByActionId =
-		std::move(m_PlayerHandGripLocalOffsetByActionId);
-	auto StagedGripLocalOffsetsByPatternId =
-		std::move(m_PlayerHandGripLocalOffsetByPatternId);
 	auto StagedEffectCues = std::move(m_PatternEffectCuesByActionId);
 	auto StagedArenaCenters = std::move(m_PatternArenaCenterAnchors);
 	auto StagedSoundCues = std::move(m_PatternSoundCuesByActionId);
@@ -1052,10 +933,6 @@
 	m_PatternClipByActionId = std::move(StagedBindings);
 	m_PatternBodyVisibilityByActionId =
 		std::move(StagedBodyVisibility);
-	m_PlayerHandGripLocalOffsetByActionId =
-		std::move(StagedGripLocalOffsetsByActionId);
-	m_PlayerHandGripLocalOffsetByPatternId =
-		std::move(StagedGripLocalOffsetsByPatternId);
 	m_PatternEffectCuesByActionId = std::move(StagedEffectCues);
 	m_PatternArenaCenterAnchors = std::move(StagedArenaCenters);
 	m_PatternSoundCuesByActionId = std::move(StagedSoundCues);
--- a/Client/Public/PlayerHandGripTransform.h
+++ b/Client/Public/PlayerHandGripTransform.h
@@ -4,14 +4,15 @@
 #include "Engine_Defines.h"
 
 #include <cmath>
-#include <cstddef>
 
 NS_BEGIN(Client)
 
-/* Authored in metres in the wrist-bone frame.  Valtan model bones can carry
-   either the imported 0.01 conversion scale or an already-normalized basis,
-   so the translation is applied through normalized hand axes in the Client's
-   metre-based world coordinates. */
+/* Authored CAPTURE hit value in metres. No Client runtime composes it onto a
+   hand bone: the grabbed player's world transform is the Server attachment
+   anchor replicated in PLAYER_SNAPSHOT and written by
+   CCharacter::Update_NetworkTransform. The struct remains the typed shape that
+   Balance Tool, Composition Detail, the encounter reference parser and the
+   gameplay publisher validate. */
 struct PLAYER_HAND_GRIP_LOCAL_OFFSET final
 {
 	f32_t fForwardM = 0.f;
@@ -21,8 +22,6 @@
 	bool operator==(const PLAYER_HAND_GRIP_LOCAL_OFFSET&) const = default;
 };
 
-/* Pure presentation math shared by replication and the headless contract
-   harness. Capture position never becomes a hand-local displacement. */
 class CPlayerHandGripTransform final
 {
 public:
@@ -36,128 +35,12 @@
 			Is_ValidGripComponent(gripLocalOffset.fRightM);
 	}
 
-	static bool_t Build_LocalOffset(
-		const float4x4_t& playerWorld,
-		const float4x4_t& handWorld,
-		float4x4_t& outLocalOffset)
-	{
-		if (!Is_UsableAffineMatrix(playerWorld) ||
-			!Is_UsableAffineMatrix(handWorld))
-			return false;
-
-		matrix_t playerBasis = DirectX::XMLoadFloat4x4(&playerWorld);
-		matrix_t handBasis = DirectX::XMLoadFloat4x4(&handWorld);
-		playerBasis.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
-		handBasis.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
-		matrix_t local = playerBasis *
-			DirectX::XMMatrixInverse(nullptr, handBasis);
-		local.r[3] = DirectX::XMVectorSet(0.f, 0.f, 0.f, 1.f);
-		float4x4_t staged{};
-		DirectX::XMStoreFloat4x4(&staged, local);
-		if (!Is_UsableAffineMatrix(staged))
-			return false;
-		outLocalOffset = staged;
-		return true;
-	}
-
-	static bool_t Compose_World(
-		const float4x4_t& localOffset,
-		const float4x4_t& handWorld,
-		float4x4_t& outWorld)
-	{
-		if (!Is_UsableAffineMatrix(localOffset) ||
-			!Is_UsableAffineMatrix(handWorld))
-			return false;
-		float4x4_t staged{};
-		DirectX::XMStoreFloat4x4(&staged,
-			DirectX::XMLoadFloat4x4(&localOffset) *
-			DirectX::XMLoadFloat4x4(&handWorld));
-		if (!Is_UsableAffineMatrix(staged))
-			return false;
-		outWorld = staged;
-		return true;
-	}
-
-	static bool_t Compose_World(
-		const float4x4_t& localOffset,
-		const float4x4_t& handWorld,
-		const PLAYER_HAND_GRIP_LOCAL_OFFSET& gripLocalOffset,
-		float4x4_t& outWorld)
-	{
-		if (!Is_UsableAffineMatrix(localOffset) ||
-			!Is_UsableAffineMatrix(handWorld) ||
-			!Is_ValidGripLocalOffset(gripLocalOffset))
-		{
-			return false;
-		}
-		if (0.f == gripLocalOffset.fForwardM &&
-			0.f == gripLocalOffset.fUpM &&
-			0.f == gripLocalOffset.fRightM)
-		{
-			return Compose_World(localOffset, handWorld, outWorld);
-		}
-
-		matrix_t adjustedHand = DirectX::XMLoadFloat4x4(&handWorld);
-		const vector_t right = DirectX::XMVector3Normalize(adjustedHand.r[0]);
-		const vector_t up = DirectX::XMVector3Normalize(adjustedHand.r[1]);
-		const vector_t forward = DirectX::XMVector3Normalize(adjustedHand.r[2]);
-		const vector_t displacement =
-			right * gripLocalOffset.fRightM +
-			up * gripLocalOffset.fUpM +
-			forward * gripLocalOffset.fForwardM;
-		adjustedHand.r[3] = DirectX::XMVectorSetW(
-			adjustedHand.r[3] + displacement, 1.f);
-
-		float4x4_t stagedHand{};
-		DirectX::XMStoreFloat4x4(&stagedHand, adjustedHand);
-		return Compose_World(localOffset, stagedHand, outWorld);
-	}
-
 private:
 	static bool_t Is_ValidGripComponent(const f32_t value)
 	{
 		return std::isfinite(value) &&
 			std::abs(value) <= MAX_GRIP_OFFSET_COMPONENT_M;
 	}
-
-	static bool_t Is_UsableAffineMatrix(const float4x4_t& value)
-	{
-		for (std::size_t row = 0u; row < 4u; ++row)
-			for (std::size_t column = 0u; column < 4u; ++column)
-				if (!std::isfinite(value.m[row][column]))
-					return false;
-		if (std::abs(value._14) > 1.e-5f ||
-			std::abs(value._24) > 1.e-5f ||
-			std::abs(value._34) > 1.e-5f ||
-			std::abs(value._44 - 1.f) > 1.e-5f)
-			return false;
-
-		// Normal Valtan hand bones carry approximately 0.01 scale. Judge
-		// basis degeneracy independently of that valid model conversion.
-		double normalized[3u][3u]{};
-		for (std::size_t row = 0u; row < 3u; ++row)
-		{
-			double lengthSquared = 0.0;
-			for (std::size_t column = 0u; column < 3u; ++column)
-			{
-				const double component = value.m[row][column];
-				lengthSquared += component * component;
-			}
-			const double length = std::sqrt(lengthSquared);
-			if (!std::isfinite(length) || !(length > 0.0))
-				return false;
-			for (std::size_t column = 0u; column < 3u; ++column)
-				normalized[row][column] = value.m[row][column] / length;
-		}
-		const double determinant =
-			normalized[0u][0u] * (normalized[1u][1u] * normalized[2u][2u] -
-				normalized[1u][2u] * normalized[2u][1u]) -
-			normalized[0u][1u] * (normalized[1u][0u] * normalized[2u][2u] -
-				normalized[1u][2u] * normalized[2u][0u]) +
-			normalized[0u][2u] * (normalized[1u][0u] * normalized[2u][1u] -
-				normalized[1u][1u] * normalized[2u][0u]);
-		return std::isfinite(determinant) && std::abs(determinant) > 1.e-6;
-	}
 };
 
 NS_END
--- a/Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
+++ b/Tools/ValtanPatternAuditionServiceHarness/Private/ValtanPatternAuditionServiceHarness.cpp
@@ -20,7 +20,6 @@
 int Run_ValtanPatternAnimationBindingDocumentContractTests();
 int Run_ValtanPatternEffectCueAuthoringContractTests();
 int Run_ValtanPresentationGenerationAdmissionContractTests();
-int Run_PlayerHandGripTransformContractTests();
 int Run_CombatDebugVisibilityContractTests();
 
 using namespace Client;
@@ -1437,8 +1436,6 @@
 		Run_ValtanPatternEffectCueAuthoringContractTests();
 	const int PresentationGenerationAdmissionFailures =
 		Run_ValtanPresentationGenerationAdmissionContractTests();
-	const int PlayerHandGripTransformFailures =
-		Run_PlayerHandGripTransformContractTests();
 	const int CombatDebugVisibilityFailures =
 		Run_CombatDebugVisibilityContractTests();
 	return 0u == Failed && 0 == FlowFailures && 0 == TuningFailures &&
@@ -1451,6 +1448,5 @@
 		0 == AnimationBindingDocumentFailures &&
 		0 == EffectCueAuthoringFailures &&
 		0 == PresentationGenerationAdmissionFailures &&
-		0 == PlayerHandGripTransformFailures &&
 		0 == CombatDebugVisibilityFailures ? 0 : 1;
 }
--- a/Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
+++ b/Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj
@@ -65,7 +65,6 @@
 	<ClCompile Include="..\Private\ActionCompositionGraphModelContractTests.cpp" />
 	<ClCompile Include="..\Private\BossLogicFlowViewModelContractTests.cpp" />
 	<ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp" />
-	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
 	<ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp" />
 	<ClCompile Include="..\Private\ValtanEncounterReferenceContractTests.cpp" />
     <ClCompile Include="..\Private\ValtanPatternFlowServiceTests.cpp" />
--- a/Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
+++ b/Tools/ValtanPatternAuditionServiceHarness/Default/ValtanPatternAuditionServiceHarness.vcxproj.filters
@@ -7,7 +7,6 @@
 	<ClCompile Include="..\Private\ActionCompositionGraphModelContractTests.cpp" />
 	<ClCompile Include="..\Private\BossLogicFlowViewModelContractTests.cpp" />
 	<ClCompile Include="..\Private\ValtanPresentationGenerationAdmissionContractTests.cpp" />
-	<ClCompile Include="..\Private\PlayerHandGripTransformContractTests.cpp" />
 	<ClCompile Include="..\Private\CombatDebugVisibilityContractTests.cpp" />
 	<ClCompile Include="..\Private\ValtanEncounterReferenceContractTests.cpp" />
     <ClCompile Include="..\Private\ValtanPatternFlowServiceTests.cpp" />
--- a/Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py
+++ b/Tools/ValtanPipeline/test_valtan_grip_local_offset_contract.py
@@ -12,6 +12,8 @@
 VALTAN_HEADER = ROOT / "Client/Public/Valtan.h"
 VALTAN_SOURCE = ROOT / "Client/Private/Valtan.cpp"
 REPLICATION_SOURCE = ROOT / "Client/Private/ClientReplication.cpp"
+CHARACTER_SOURCE = ROOT / "Client/Private/Character.cpp"
+GAME_ROOM_SOURCE = ROOT / "Server/Private/GameRoom.cpp"
 
 
 def load_gameplay() -> dict:
@@ -58,23 +60,29 @@
             set(bindings),
         )
 
-    def test_client_lookup_is_pattern_keyed_and_fail_closed(self) -> None:
+    def test_client_never_composes_the_grip_on_a_hand_bone(self) -> None:
         header = VALTAN_HEADER.read_text(encoding="utf-8-sig")
         valtan = VALTAN_SOURCE.read_text(encoding="utf-8-sig")
         replication = REPLICATION_SOURCE.read_text(encoding="utf-8-sig")
-        self.assertIn("m_PlayerHandGripLocalOffsetByPatternId", header)
-        self.assertIn("Try_Get_PlayerHandGripLocalOffsetByPatternId", header)
-        self.assertIn(
-            "Pattern has conflicting CAPTURE gripLocalOffset values", valtan
-        )
-        self.assertIn("Get_ServerPatternId()", replication)
-        self.assertIn(
-            "bHasGripLocalOffset =\n"
-            "\t\t\t\tvaltan->Try_Get_PlayerHandGripLocalOffsetByPatternId(",
-            replication,
-        )
-        self.assertIn("retained its Server fallback", replication)
-        self.assertNotIn("(void)valtan->Try_Get_PlayerHandGripLocalOffset(", replication)
+        character = CHARACTER_SOURCE.read_text(encoding="utf-8-sig")
+        game_room = GAME_ROOM_SOURCE.read_text(encoding="utf-8-sig")
+        for forbidden in (
+            "Update_PlayerAttachmentPresentations",
+            "Stage_PlayerAttachmentPresentation",
+            "bip001-l-hand",
+            "m_PlayerAttachments",
+        ):
+            self.assertNotIn(forbidden, replication)
+        for forbidden in (
+            "m_PlayerHandGripLocalOffsetByPatternId",
+            "Try_Get_PlayerHandGripLocalOffset",
+            "Reload_PlayerHandGripLocalOffsets_WhileAdmitted",
+        ):
+            self.assertNotIn(forbidden, header)
+            self.assertNotIn(forbidden, valtan)
+        self.assertIn("PLAYER_ACTION_STATE::GRABBED == action", character)
+        self.assertIn("Update_PlayerAttachment(player, updateTick)", game_room)
+        self.assertIn("player.fAttachmentLocalOffsetX * cosine", game_room)
 
     def test_missing_grip_rejects_without_mutating_committed_source(self) -> None:
         committed = load_gameplay()
--- a/Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
+++ b/Data/Effects/V2/Bindings/BOSS_VALTAN.effectv2bindings.json
@@ -1723,8 +1723,9 @@
           ]
         }
       },
-      "stopPolicy": "NATURAL"
+      "stopPolicy": "STAGE_END"
     },
+    { "bindingId": "binding.valtan.project-tuned.stagger-slot.channel.aura", "resource": { "kind": "GROUP", "id": "boss.valtan.magicball.aura" }, "scope": { "patternId": "VALTAN_STAGGER_SLOT", "stageId": "CHANNEL", "actionId": "valtan.authoring.stagger-slot.channel" }, "clock": { "basis": "STAGE", "clipOccurrenceId": null, "startMs": 0, "repeatPolicy": "ONCE" }, "anchor": { "slotId": "b_effectroot", "followPolicy": "FOLLOW_SLOT", "rotationBasis": "TARGET_YAW", "localTransform": { "translation": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0] } }, "stopPolicy": "STAGE_END" },
     { "bindingId": "binding.valtan.project-tuned.bind-slot.stomp", "resource": { "kind": "GROUP", "id": "boss.valtan.impact" }, "scope": { "patternId": "VALTAN_BIND_SLOT", "stageId": "STEP_01", "actionId": "valtan.authoring.bind-slot.step-01" }, "clock": { "basis": "CLIP_OCCURRENCE", "clipOccurrenceId": "VALTAN_BIND_SLOT.STEP_01.composition.clip.01", "startMs": 1200, "repeatPolicy": "ONCE" }, "anchor": { "slotId": "b_effectroot", "followPolicy": "SNAPSHOT_AT_START", "rotationBasis": "TARGET_YAW", "localTransform": { "translation": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0] } }, "stopPolicy": "NATURAL" },
     { "bindingId": "binding.valtan.project-tuned.counter-pulse.001", "resource": { "kind": "GROUP", "id": "boss.valtan.project-tuned.sequence.trash.pulse-group" }, "scope": { "patternId": "VALTAN_TRASH", "stageId": "STEP_07", "actionId": "valtan.sequence.center-trash-rush-if.step-07" }, "clock": { "basis": "CLIP_OCCURRENCE", "clipOccurrenceId": "valtan.sequence.center-trash-rush-if.step-07.clip-01", "startMs": 0, "repeatPolicy": "ONCE" }, "anchor": { "slotId": "b_effectroot", "followPolicy": "FOLLOW_SLOT", "rotationBasis": "TARGET_YAW", "localTransform": { "translation": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0] } }, "stopPolicy": "NATURAL" },
     { "bindingId": "binding.valtan.project-tuned.counter-pulse.002", "resource": { "kind": "GROUP", "id": "boss.valtan.project-tuned.sequence.trash.pulse-group" }, "scope": { "patternId": "VALTAN_TRASH", "stageId": "STEP_07", "actionId": "valtan.sequence.center-trash-rush-if.step-07" }, "clock": { "basis": "CLIP_OCCURRENCE", "clipOccurrenceId": "valtan.sequence.center-trash-rush-if.step-07.clip-01", "startMs": 200, "repeatPolicy": "ONCE" }, "anchor": { "slotId": "b_effectroot", "followPolicy": "FOLLOW_SLOT", "rotationBasis": "TARGET_YAW", "localTransform": { "translation": [0.0, 0.0, 0.0], "rotation": [0.0, 0.0, 0.0], "scale": [1.0, 1.0, 1.0] } }, "stopPolicy": "NATURAL" },
--- a/.md/TEAM/발탄인수인계서.md
+++ b/.md/TEAM/발탄인수인계서.md
@@ -1738,8 +1738,11 @@
 `_CATCH_SUCCESS`는 같은 성공 tail, `_CATCH_FAIL`은 같은 miss tail을 공유한다. 보조 success를 실행했다고
 존재하지 않는 포획자를 만들지 않는다. 네 패턴의 모든 stage graph는 back-edge 없이 유한해야 한다.
 
-Client grab은 왼손 본의 고정 local origin에 붙인다. capture 시점의 상대 위치는 보존하지 않고 회전과
-player scale만 보존한다. 실제 0.01 hand scale도 허용하도록 정규화한 basis determinant를 검사한다.
+Client grab은 손 bone 합성 없이 Server attachment anchor를 그대로 쓴다. Server는 capture 순간의
+boss-local 상대 위치와 yaw 오프셋을 저장해 매 tick `boss pos + yaw 회전(offset)`을 플레이어 위치로
+복제하고, Client `CCharacter::Update_NetworkTransform`이 그 값을 보간해 transform에 쓴다. 본체,
+nameplate, Debug wire, Server 판정이 같은 좌표를 사용한다. `gripLocalOffset`은 저작 데이터로 남지만
+runtime 소비자는 없다.
 `CATCH_BREATH`의 `ARENA_EJECTION`은 기존 12m/s를 24m/s로 두 배 늘린다. `yawOffsetDegrees`는
 Server가 소유한 기존 backward release 방향에 더하는 `-180..180` 상대 yaw이며 기본값 0도다. 180도는
 그 결과를 boss facing 정면 방향으로 뒤집는다. Animation Workbench는
--- a/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md
+++ b/.md/TEAM/TEAM_GAMEPLAY_INTERFACE_HANDBOOK.md
@@ -561,9 +561,10 @@
 
 실행 중 boss pattern이 한 플레이어를 잠그면 Server의 stable `NetEntityId`가
 `WORLD_ENTITY_SNAPSHOT::iPatternTargetNetEntityId`로 복제된다. 일반 NPC와 monster는 이
-필드를 반드시 invalid로 둔다. Client는 대상을 다시 고르지 않으며, 62줄 잡기 표현은 이 ID로
-찾은 Character만 `bip001-l-hand` animated socket에 붙였다가 success/recovery 또는 패턴 종료 시
-Server snapshot transform으로 돌려놓는다.
+필드를 반드시 invalid로 둔다. Client는 대상을 다시 고르지 않는다. 잡힌 Character의 transform은
+`PLAYER_SNAPSHOT`의 GRABBED 상태와 Server attachment anchor 위치를
+`CCharacter::Update_NetworkTransform`이 보간한 값이며, Client가 boss 손 bone에 다시 붙이지 않는다.
+release 뒤에도 같은 snapshot 경로가 이어진다.
 
 ```text
 CGameRoom::Tick
--- a/Tools/ValtanPatternAuditionServiceHarness/Private/PlayerHandGripTransformContractTests.cpp
+++ /dev/null
@@ -1,200 +0,0 @@
-#include "PlayerHandGripTransform.h"
-
-#include <array>
-#include <cmath>
-#include <iostream>
-#include <limits>
-#include <stdexcept>
-
-namespace
-{
-	constexpr f32_t EPSILON = 1.e-4f;
-
-	void Require(const bool_t condition, const char* const message)
-	{
-		if (!condition)
-			throw std::runtime_error(message);
-	}
-
-	bool_t MatricesNear(
-		const float4x4_t& left,
-		const float4x4_t& right)
-	{
-		for (std::size_t row = 0u; row < 4u; ++row)
-			for (std::size_t column = 0u; column < 4u; ++column)
-				if (std::abs(left.m[row][column] - right.m[row][column]) >
-					EPSILON)
-				{
-					return false;
-				}
-		return true;
-	}
-
-	float4x4_t Store(const matrix_t value)
-	{
-		float4x4_t stored{};
-		DirectX::XMStoreFloat4x4(&stored, value);
-		return stored;
-	}
-
-	void VerifyH3CaptureBasisAndDistanceContract()
-	{
-		const float4x4_t handWorld = Store(
-			DirectX::XMMatrixScaling(0.01f, 0.01f, 0.01f) *
-			DirectX::XMMatrixRotationRollPitchYaw(0.4f, -0.8f, 0.2f) *
-			DirectX::XMMatrixTranslation(5.f, 3.f, -7.f));
-		const float4x4_t playerWorld = Store(
-			DirectX::XMMatrixScaling(1.25f, 0.75f, 1.5f) *
-			DirectX::XMMatrixRotationRollPitchYaw(0.2f, 0.6f, -0.1f) *
-			DirectX::XMMatrixTranslation(40.f, 2.f, -10.f));
-		float4x4_t distantPlayerWorld = playerWorld;
-		distantPlayerWorld._41 = -80.f;
-		distantPlayerWorld._42 = 7.f;
-		distantPlayerWorld._43 = 90.f;
-
-		float4x4_t localOffset{};
-		float4x4_t distantLocalOffset{};
-		Require(Client::CPlayerHandGripTransform::Build_LocalOffset(
-			playerWorld, handWorld, localOffset),
-			"H3 local offset rejected the normal 0.01 hand scale");
-		Require(Client::CPlayerHandGripTransform::Build_LocalOffset(
-			distantPlayerWorld, handWorld, distantLocalOffset),
-			"H3 local offset rejected the distant capture fixture");
-		Require(MatricesNear(localOffset, distantLocalOffset) &&
-			0.f == localOffset._41 && 0.f == localOffset._42 &&
-			0.f == localOffset._43,
-			"H3 local offset retained capture distance");
-
-		float4x4_t attachedWorld{};
-		Require(Client::CPlayerHandGripTransform::Compose_World(
-			localOffset, handWorld, attachedWorld),
-			"H3 local times hand composition failed");
-		float4x4_t expectedWorld = playerWorld;
-		expectedWorld._41 = handWorld._41;
-		expectedWorld._42 = handWorld._42;
-		expectedWorld._43 = handWorld._43;
-		Require(MatricesNear(attachedWorld, expectedWorld),
-			"H3 composition changed player basis or retained Valtan scale");
-	}
-
-	void VerifyAuthoredGripUsesNormalizedHandAxes()
-	{
-		const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET grip{
-			0.25f, -0.9f, 0.1f };
-		float4x4_t identity{};
-		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
-		float4x4_t reference{};
-		bool_t hasReference = false;
-		for (const f32_t handScale : { 0.01f, 0.009999995f, 1.f })
-		{
-			const float4x4_t handWorld = Store(
-				DirectX::XMMatrixScaling(handScale, handScale, handScale) *
-				DirectX::XMMatrixRotationY(DirectX::XM_PIDIV2) *
-				DirectX::XMMatrixTranslation(10.f, 20.f, 30.f));
-			float4x4_t attached{};
-			Require(Client::CPlayerHandGripTransform::Compose_World(
-				identity, handWorld, grip, attached),
-				"authored grip rejected a valid normal or ghost hand scale");
-
-			const matrix_t hand = DirectX::XMLoadFloat4x4(&handWorld);
-			const vector_t displacement =
-				DirectX::XMVector3Normalize(hand.r[0]) *
-					grip.fRightM +
-				DirectX::XMVector3Normalize(hand.r[1]) *
-					grip.fUpM +
-				DirectX::XMVector3Normalize(hand.r[2]) *
-					grip.fForwardM;
-			float3_t expectedDisplacement{};
-			DirectX::XMStoreFloat3(&expectedDisplacement, displacement);
-			Require(std::abs(attached._41 -
-				(handWorld._41 + expectedDisplacement.x)) <= EPSILON &&
-				std::abs(attached._42 -
-					(handWorld._42 + expectedDisplacement.y)) <= EPSILON &&
-				std::abs(attached._43 -
-					(handWorld._43 + expectedDisplacement.z)) <= EPSILON,
-				"authored metre offset was not applied in normalized hand axes");
-			Require(std::abs(attached._42 - (handWorld._42 - 0.9f)) <= EPSILON,
-				"authored -0.9 metre grip moved by a non-metre world distance");
-			if (!hasReference)
-			{
-				reference = attached;
-				hasReference = true;
-			}
-			else
-			{
-				Require(std::abs(attached._41 - reference._41) <= EPSILON &&
-					std::abs(attached._42 - reference._42) <= EPSILON &&
-					std::abs(attached._43 - reference._43) <= EPSILON,
-					"grip translation was multiplied by imported bone scale");
-			}
-		}
-
-		float4x4_t legacy{};
-		float4x4_t missingFallback{};
-		const float4x4_t handWorld = Store(
-			DirectX::XMMatrixRotationY(0.3f) *
-			DirectX::XMMatrixTranslation(4.f, 5.f, 6.f));
-		Require(Client::CPlayerHandGripTransform::Compose_World(
-			identity, handWorld, legacy) &&
-			Client::CPlayerHandGripTransform::Compose_World(
-				identity, handWorld,
-				Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, missingFallback) &&
-			MatricesNear(legacy, missingFallback),
-			"missing optional grip offset changed the legacy H3 contract");
-	}
-
-	void VerifyInvalidInputRollsBackOutput()
-	{
-		float4x4_t identity{};
-		DirectX::XMStoreFloat4x4(&identity, DirectX::XMMatrixIdentity());
-		const float4x4_t committed = Store(
-			DirectX::XMMatrixTranslation(3.f, 4.f, 5.f));
-		for (const Client::PLAYER_HAND_GRIP_LOCAL_OFFSET invalid : {
-			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{
-				(std::numeric_limits<f32_t>::quiet_NaN)(), 0.f, 0.f },
-			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{
-				0.f, (std::numeric_limits<f32_t>::infinity)(), 0.f },
-			Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{ 0.f, 0.f, 10.01f } })
-		{
-			float4x4_t unchanged = committed;
-			Require(!Client::CPlayerHandGripTransform::Compose_World(
-				identity, identity, invalid, unchanged) &&
-				MatricesNear(unchanged, committed),
-				"invalid grip offset changed committed presentation output");
-		}
-
-		std::array<float4x4_t, 3u> invalidMatrices{
-			identity, identity, identity };
-		invalidMatrices[0]._11 = 0.f;
-		invalidMatrices[1]._41 =
-			(std::numeric_limits<f32_t>::quiet_NaN)();
-		invalidMatrices[2]._14 = 1.f;
-		for (const float4x4_t& invalid : invalidMatrices)
-		{
-			float4x4_t unchanged = committed;
-			Require(!Client::CPlayerHandGripTransform::Compose_World(
-				identity, invalid,
-				Client::PLAYER_HAND_GRIP_LOCAL_OFFSET{}, unchanged) &&
-				MatricesNear(unchanged, committed),
-				"invalid hand matrix changed committed presentation output");
-		}
-	}
-}
-
-int Run_PlayerHandGripTransformContractTests()
-{
-	try
-	{
-		VerifyH3CaptureBasisAndDistanceContract();
-		VerifyAuthoredGripUsesNormalizedHandAxes();
-		VerifyInvalidInputRollsBackOutput();
-		std::cout << "PlayerHandGripTransformContractTests: 3/3 passed\n";
-		return 0;
-	}
-	catch (const std::exception& error)
-	{
-		std::cerr << "PlayerHandGripTransformContractTests: FAIL: " <<
-			error.what() << '\n';
-		return 1;
-	}
-}
````
