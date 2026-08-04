# 2026-08-04 Dimensionist playable vertical slice PLAN

## 0. 목적과 완료 기준

차원술사(`DIMENSIONIST`)를 Debug Animation Tool 전용 미리보기에서 정식 지원 플레이어 클래스로 승격한다. 기존 `CModel -> CMaterial`, `CPlayableCharacterAssetService`, `CCharacterCatalog`, `CClientReplication`, 서버 승인 입장 경로만 확장하며 별도 NetworkClient·별도 모델 런타임·Client 로컬 스킬 우회를 만들지 않는다.

완료 기준은 다음과 같다.

1. `CHARACTER_CLASS_ID::DIMENSIONIST`가 기존 wire 값을 바꾸지 않는 stable ID `5`로 직렬화되며 프로토콜 버전 `6`에서만 교환된다.
2. Character Select가 다섯 번째 항목으로 차원술사를 표시하고 `Dimensionist_Character.wmodel`을 미리보기한 뒤 선택 상태를 저장한다.
3. 서버가 `PlayerProfiles.json`의 차원술사 프로필을 publish/load하고 입장 승인 뒤 동일 enum을 `S2C_PLAYER_SPAWNED`로 되돌린다.
4. `CClientReplication`이 기존 공통 경로에서 차원술사 `CCharacter`를 생성하고 제품 locomotion/death 경로에서 IDLE/RUN/DEAD를 재생한다. HIT는 WModel/Animation Tool에서 resolve하지만 현재 snapshot action enum에 HIT 의미가 없으므로 제품 자동 재생 완료로 주장하지 않는다.
5. Animation Tool이 `Dimensionist` target과 `Data/Animation/Authored/Dimensionist`를 같은 이름으로 찾는다.
6. Effect Tool이 `Data/Effects/Authored/Dimensionist/Candidates`의 459개 `.effect`와 `Resources/Effect/Dimensionist` 의존 에셋을 열 수 있다. 후보의 `candidate_only` admission은 유지하며 미완성 파티클을 제품 스킬에 자동 연결하지 않는다.
7. NetworkProtocolHarness, Server contract test, Client Debug/Release build와 ProjectAudit를 실행하고 결과를 RESULT에 기록한다. 사용자가 폐기한 resource pack `2026.08.03.4`는 다시 만들지 않는다.

## 1. 실측 원인

- `Client/Bin/Resources/Character/Dimensionist/Dimensionist_Character.wmodel`은 skeleton 1개, mesh 10개, animation 154개를 가진 결합형 본체다.
- 런타임 기본 클립은 실제 WModel 이름 `pc_sp_m_00_sk_idle_battle_1`, `pc_sp_m_00_sk_run_battle_1`, `pc_sp_m_00_sk_dmg_idle_1`, `pc_sp_m_00_sk_dead_1`로 확인됐다.
- 현재 `CPlayableCharacterAssetService`는 `equipmentModels.size() == 5`를 강제하고 weapon을 무조건 stage한다.
- 현재 `CActorCatalog`는 `runtimeStatus == supported`를 `equipmentModels` 비어 있지 않음과 `weaponModel` 비어 있지 않음으로 정의한다.
- 따라서 차원술사를 카탈로그에 추가해도 body-only 결합형 모델은 prototype admission 이전에 거부된다.
- 네트워크 packet shape에는 이미 `CHARACTER_CLASS_ID`가 있으므로 새 packet/새 NetworkClient 구조체는 필요 없다. 서버의 `SERVER_PLAYER::eCharacterClass`와 기존 spawn message가 정본이다.

## 2. 변경 계약

### 2.1 Shared wire enum

`Shared/Public/Network/PacketType.h`의 관련 블록을 다음으로 교체한다.

```cpp
inline constexpr std::uint16_t NETWORK_PROTOCOL_VERSION = 6;

enum class CHARACTER_CLASS_ID : std::uint8_t
{
	LANCE_MASTER = 0,
	GUNSLINGER = 1,
	SLAYER = 2,
	ARTIST = 3,
	DESTROYER = 4,
	DIMENSIONIST = 5,
	END
};

[[nodiscard]]
constexpr bool Is_Supported_Playable_Character_Class(
	const CHARACTER_CLASS_ID characterClass)
{
	return CHARACTER_CLASS_ID::LANCE_MASTER == characterClass ||
		CHARACTER_CLASS_ID::GUNSLINGER == characterClass ||
		CHARACTER_CLASS_ID::SLAYER == characterClass ||
		CHARACTER_CLASS_ID::ARTIST == characterClass ||
		CHARACTER_CLASS_ID::DIMENSIONIST == characterClass;
}
```

`DESTROYER = 4`를 보존하고 새 값만 뒤에 추가한다. `END` 경계가 바뀌므로 구버전 peer와의 의미 호환을 허용하지 않고 protocol을 6으로 올린다.

### 2.2 Actor catalog의 결합형/모듈형 표현

`Data/Actors/CharacterCatalog.json`에 다음 레코드를 추가한다.

```json
{
  "archetypeId": "PLAYER_DIMENSIONIST",
  "networkClassId": "DIMENSIONIST",
  "assetId": "Dimensionist",
  "bodyModel": "Character/Dimensionist/Dimensionist_Character.wmodel",
  "equipmentModels": [],
  "weaponModel": null,
  "animationSetId": "ANIM_DIMENSIONIST",
  "runtimeStatus": "supported"
}
```

`Client/Private/ActorCatalog.cpp::ParseClass`에 다음 case를 추가한다.

```cpp
if (value == "DIMENSIONIST") return CHARACTER_CLASS_ID::DIMENSIONIST;
```

지원 클래스의 presentation 조합 검증은 다음 완전한 조건으로 교체한다.

```cpp
const bool_t hasEquipment = !entry.equipmentModels.empty();
const bool_t hasWeapon = !entry.weaponModel.empty();
const bool_t hasCompleteModularPresentation = hasEquipment && hasWeapon;
const bool_t hasCompleteCombinedPresentation = !hasEquipment && !hasWeapon;
if (entry.runtimeStatus == "supported" &&
	!hasCompleteModularPresentation &&
	!hasCompleteCombinedPresentation)
{
	return false;
}
if (entry.runtimeStatus == "reserved" &&
	(hasEquipment || hasWeapon))
{
	return false;
}
```

즉 지원 클래스는 `body + equipment + weapon` 또는 `combined body only` 중 하나만 허용하고 반쪽 조합을 거부한다.

### 2.3 Prototype admission

`Client/Private/PlayableCharacterAssetService.cpp`의 tag 구조체를 다음으로 교체한다.

```cpp
struct CHARACTER_PROTOTYPE_TAGS final
{
	const tchar_t* pBody = nullptr;
	std::array<const tchar_t*, 5> Equipment{};
	size_t iEquipmentCount = 0;
	const tchar_t* pWeapon = nullptr;
};
```

기존 네 static tag 레코드에는 equipment count `5u`를 추가하고 차원술사 레코드를 추가한다.

```cpp
static const CHARACTER_PROTOTYPE_TAGS DIMENSIONIST
{
	TEXT("Prototype_Component_Model_Dimensionist"),
	{},
	0u,
	nullptr
};
```

`Find_Tags`에 다음 case를 추가한다.

```cpp
case LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONIST:
	return &DIMENSIONIST;
```

admission 검증·reserve·장비 loop·무기 stage를 다음 계약으로 교체한다.

```cpp
if (nullptr == pActor || nullptr == pTags ||
	pActor->runtimeStatus != "supported")
{
	return E_FAIL;
}
const bool_t hasWeapon = !pActor->weaponModel.empty();
if (
	pActor->equipmentModels.size() != pTags->iEquipmentCount ||
	hasWeapon != (nullptr != pTags->pWeapon))
{
	return E_FAIL;
}

std::vector<std::pair<std::wstring, unique_ptr<CPrototype>>> staged;
staged.reserve(1u + pTags->iEquipmentCount + (hasWeapon ? 1u : 0u));

for (size_t index = 0; index < pTags->iEquipmentCount; ++index)
{
	if (Is_Cancelled(pCancellationRequested))
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (!StageModel(
		pTags->Equipment[index],
		pActor->equipmentModels[index],
		MODEL::ANIM,
		characterTransform))
	{
		return E_FAIL;
	}
}

if (hasWeapon)
{
	if (Is_Cancelled(pCancellationRequested))
		return HRESULT_FROM_WIN32(ERROR_CANCELLED);
	if (!StageModel(
		pTags->pWeapon,
		pActor->weaponModel,
		MODEL::NONANIM,
		XMMatrixIdentity()))
	{
		return E_FAIL;
	}
}
```

차원술사는 staged prototype이 body 하나이므로 `Add_Prototype` 성공/실패가 원자적이다. 비평에서 기존 네 클래스의 다중 파츠 순차 commit에 일반적인 batch API가 없다는 문제가 확인됐지만, 이는 이번 body-only class에 부분 commit을 만들지 않는다. 기존 네 클래스의 범용 Engine batch admission은 별도 Engine public 계약 변경으로 남긴다.

### 2.4 Dimensionist Character spec

새 `Client/Public/Logic_Dimensionist.h` 전체 내용:

```cpp
#pragma once

#include "Client_Defines.h"
#include "CharacterSpec.h"

NS_BEGIN(Client)

class CLogic_Dimensionist final : public ICharacterLogic
{
public:
	virtual void Update_Presentation(
		CCharacter& Character,
		f32_t fTimeDelta) override;
};

extern const CHARACTER_SPEC Spec_Dimensionist;

NS_END
```

새 `Client/Private/Logic_Dimensionist.cpp` 전체 내용:

```cpp
#include "Logic_Dimensionist.h"

#include "Character.h"

namespace
{
	unique_ptr<ICharacterLogic> Create_Logic()
	{
		return make_unique<Client::CLogic_Dimensionist>();
	}
}

NS_BEGIN(Client)

void CLogic_Dimensionist::Update_Presentation(
	CCharacter& Character,
	f32_t fTimeDelta)
{
	(void)Character;
	(void)fTimeDelta;
}

const CHARACTER_SPEC Spec_Dimensionist =
{
	"Dimensionist",
	LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONIST,

	TEXT("Prototype_Component_Model_Dimensionist"),
	TEXT("Prototype_Component_Shader_VtxAnimMeshBinary"),
	0u,

	nullptr,
	nullptr,
	0u,

	nullptr,
	0u,

	{
		"pc_sp_m_00_sk_idle_battle_1",
		"pc_sp_m_00_sk_run_battle_1",
		"pc_sp_m_00_sk_dmg_idle_1",
		"pc_sp_m_00_sk_dead_1",
	},

	&Create_Logic,
};

NS_END
```

`CharacterCatalog.cpp`는 header include와 다음 switch case를 추가한다.

```cpp
#include "Logic_Dimensionist.h"

case CHARACTER_CLASS_ID::DIMENSIONIST:
	return &Spec_Dimensionist;
```

### 2.5 Character Select와 로딩

`Client/Public/Level_CharacterSelect.h`의 roster를 다음으로 교체한다.

```cpp
static constexpr std::array<
	LostArk::Shared::CHARACTER_CLASS_ID, 5> SUPPORTED_CLASSES =
{
	LostArk::Shared::CHARACTER_CLASS_ID::LANCE_MASTER,
	LostArk::Shared::CHARACTER_CLASS_ID::GUNSLINGER,
	LostArk::Shared::CHARACTER_CLASS_ID::SLAYER,
	LostArk::Shared::CHARACTER_CLASS_ID::ARTIST,
	LostArk::Shared::CHARACTER_CLASS_ID::DIMENSIONIST
};
```

`Loader.cpp::Ready_For_CharacterSelect`의 array에도 같은 다섯 값을 둔다. `Level_CharacterSelect.cpp`, `Level_Lobby.cpp`, `MainApp.cpp`의 class name switch에는 다음 case를 추가한다.

```cpp
case CHARACTER_CLASS_ID::DIMENSIONIST:
	return "Dimensionist";
```

`PlayerSkillCatalog.cpp` parser에도 다음 줄을 추가하되 아직 서버 승인 skill 정의는 만들지 않는다.

```cpp
if (value == "DIMENSIONIST") return CHARACTER_CLASS_ID::DIMENSIONIST;
```

### 2.6 Server profile와 기존 구조체 소비

`Data/Balance/PlayerProfiles.json`에 다음 레코드를 추가한다.

```json
{
  "characterClass": "DIMENSIONIST",
  "maximumHp": 5000,
  "maximumResource": 100,
  "moveSpeed": 6.0
}
```

`Tools/GameplayPipeline/Publish-GameplayBalance.ps1`의 지원 목록을 교체한다.

```powershell
$supportedPlayerClasses = @(
	'LANCE_MASTER',
	'GUNSLINGER',
	'SLAYER',
	'ARTIST',
	'DIMENSIONIST'
)
```

`Server/Private/GameplayCatalog.cpp::ParseCharacterClass`에 다음 branch를 추가한다.

```cpp
else if ("DIMENSIONIST" == value)
	output = CHARACTER_CLASS_ID::DIMENSIONIST;
```

별도 차원술사 서버 구조체는 추가하지 않는다. `SERVER_PLAYER::eCharacterClass`, `PLAYER_RUNTIME_PROFILE`, `C2S_ENTER_WORLD`, `S2C_PLAYER_SPAWNED`가 이미 필요한 typed 계약을 소유하며 Server에 asset path나 animation name을 넣지 않는다.

### 2.7 Animation Tool 데이터

현재 추출물에는 PSA animation 154개와 프레임/fps 정보는 있지만 Action `.loa` notify/skill 매핑 원본은 없다. 존재하지 않는 skill ID·타이밍·effect notify를 추측하지 않는다. Tool이 기존 클래스와 같은 경로를 즉시 열 수 있도록 authored 문서와 reference 문서 4종을 각 포맷의 0-row 정식 컨테이너로 만든다. 이 파일 존재는 경로/파서 계약 증거일 뿐 추출된 스킬 데이터 증거가 아니다.

`Data/Animation/Authored/Dimensionist/Dimensionist.animevents`:

```text
LOSTARK_ANIM_EVENTS 3 "Dimensionist" 0
```

Animation Tool의 clip 목록 자체는 WModel의 154개 animation section에서 온다. reference 컨테이너는 다음 헤더만 가지며 row는 0개다.

```text
LOSTARK_ANIM_NOTIFY 1 "Dimensionist" 0
LOSTARK_CLIP_MAP 1 "Dimensionist" 0
LOSTARK_CLIP_SEQ 2 "Dimensionist" 0
LOSTARK_SKILL_TIMING 2 "Dimensionist" 0
```

Action 원본이 확보되기 전에는 이 문서에 skill row를 추가하지 않고, 0-row 컨테이너를 유효한 추출 reference나 스킬 완료 증거로 해석하지 않는다.

### 2.8 Effect admission

이미 준비된 다음 정본을 유지한다.

- `Data/Effects/SourceCatalog/dimensionist_admission.json`: `candidate_only`
- `Data/Effects/SourceCatalog/dimensionist_candidates.json`: 459 candidates
- `Data/Effects/Authored/Dimensionist/Candidates/*.effect`: Effect Tool authoring text
- `Client/Bin/Resources/Effect/Dimensionist`: DDS 693개, WModel 139개

`.effect`는 JSON이나 `.wfx`가 아니라 현재 Effect Tool의 line-oriented authoring text다. 제품 배포용 cook은 `.weffect` binary다. unresolved Cascade import와 disabled emitter가 남아 있으므로 이 변경에서는 스킬/animation notify에 자동 연결하지 않는다.

## 3. 프로젝트·하네스 등록

`Client/Default/Client.vcxproj`와 `.filters`에 다음을 등록한다.

```xml
<ClInclude Include="..\Public\Logic_Dimensionist.h" />
<ClCompile Include="..\Private\Logic_Dimensionist.cpp" />
<None Include="..\..\Data\Animation\Authored\Dimensionist\Dimensionist.animevents" />
<None Include="..\..\Data\Animation\Reference\Dimensionist\Dimensionist.animnotify" />
<None Include="..\..\Data\Animation\Reference\Dimensionist\Dimensionist.clipmap" />
<None Include="..\..\Data\Animation\Reference\Dimensionist\Dimensionist.clipseq" />
<None Include="..\..\Data\Animation\Reference\Dimensionist\Dimensionist.skilltiming" />
```

filter는 기존 class별 GameObject filter와 `96.DataFiles\Animation\Authored`, `96.DataFiles\Animation\Reference` 배치를 그대로 따른다.

`NetworkProtocolHarness::Test_PlayableCharacterRoster`는 다섯 지원 클래스와 `DESTROYER/END` 거부를 검증한다. enter/spawn roundtrip 중 하나는 `DIMENSIONIST`를 사용해 raw value 5가 보존되는지 검증한다.

`ServerGameplayContractTests`는 `Find_Player(DIMENSIONIST)`가 non-null이고 `Find_Player(DESTROYER)`가 null임을 검증한다.

`ProjectAudit`는 다음을 검사한다.

- Character Select loader/source에 DIMENSIONIST가 존재한다.
- actor roster가 `LANCE_MASTER,GUNSLINGER,SLAYER,ARTIST,DIMENSIONIST`이고 모두 supported다.
- 차원술사 body WModel, authored 문서와 0-row reference 컨테이너 4종이 존재한다.

Effect candidate와 local runtime dependency는 사용자가 폐기한 immutable pack `.4`에 포함되지 않고 Git 정본도 아니므로 ProjectAudit의 clean-checkout 필수 조건으로 승격하지 않는다. 로컬 추출 결과와 Effect Tool open/roundtrip은 RESULT의 수동·로컬 증거로 분리한다.

## 4. 검증 순서

1. `ModelAssetConverter.exe info Dimensionist_Character.wmodel`: 154 animations/skeleton 확인
2. JSON parse: CharacterCatalog, PlayerProfiles, effect admission/candidate catalog
3. `Publish-GameplayBalance.ps1`
4. Shared + NetworkProtocolHarness Debug/Release build와 harness 실행
5. Server Debug/Release build와 `Server.exe --contract-test`
6. Client Debug/Release build
7. Debug Client 수동 smoke: `Framework.slnLaunch`의 Server + Client -> Lobby -> Character Select -> Dimensionist preview -> confirm -> Lobby -> Test 진입, Animation Tool target/clip playback. Local Preview 경로는 없다.
8. Effect Tool: Dimensionist candidate open과 texture/model path resolve
9. `Tools/ProjectAudit/Invoke-ProjectAudit.ps1` (폐기된 pack `.4`를 재생성하지 않으므로 기존 `.3` immutable lock 기준만 사용)
10. `git diff --check`

기존 immutable pack `2026.08.03.3`에는 Dimensionist payload가 없다. 따라서 이번 수직 슬라이스의 로컬 실행 검증과 별개로, 다른 PC의 `Hydrate -> Verify`만으로 Dimensionist를 재현하는 팀 배포 계약은 사용자가 `.4` 제작을 폐기한 결정에 따라 미완료로 남긴다.

## 5. 비범위

- 검증되지 않은 차원술사 skill ID, damage, quick slot, animation clip chain
- candidate-only effect의 자동 runtime 재생
- 초월기 컷씬/camera cue/RenderTarget 캡처
- 신규 server class-specific 구조체 또는 asset path
- resource pack `2026.08.03.4`, ZIP, hydrate 배포
- 기존 네 클래스의 파츠/무기 구조 변경
