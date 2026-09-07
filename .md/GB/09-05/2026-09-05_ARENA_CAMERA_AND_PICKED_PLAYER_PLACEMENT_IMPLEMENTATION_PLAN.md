# 2026-09-05 아레나 카메라와 피킹 위치 플레이어 배치 구현 계획

최초 기준은 PR #316 충돌 해결 commit `d3d9a21e`이며 작업 브랜치는 `codex/arena-camera-player-placement`다.
해당 PR이 병합된 `origin/main`의 `96f907b2`는 같은 tree이므로 최종 커밋은 이 main 위에 정리한다.
원본 작업 폴더의 다른 작업과 실행 중인 Client/Server는 보존한다.

## G00. 아레나별 자유 카메라 속도

발탄은 camera span 180에서 `max(20, span * 0.08)`을 적용해 기본 20m/s다.
쿠크는 같은 식에 거대한 맵 범위를 넣어 훨씬 빨라진다. 두 아레나의 기본값을 20m/s로 맞춘다.

`CCamera_Free`는 유한한 양수 속도를 검증하고 기존 Transform 이동량에 비율로 적용한다.
각 Level은 현재 아레나 카메라와 process-session 속도를 소유한다. F1의 현재 아레나 항목에서
즉시 조절·기본값 복원을 제공하고 F6 전환 및 같은 process의 재진입에서 선택을 유지한다.
Shift의 기존 30배 이동은 표시한다. 이 요청에는 디스크 저장을 추가하지 않는다.

변경 위치는 Camera_Free H/CPP와 두 Arena Level의 camera 생성·접근 함수, MainApp F1 표시다.
Engine Transform과 컷신 카메라 시계는 변경하지 않는다.

## G01. F6 상태에서 명시적으로 지면을 선택해 플레이어 배치

F1 `Move Player`를 누르면 마우스 회전을 끄고 지면 선택을 한 번 대기한다. UI 밖의 새 좌클릭만
기존 world-position picking으로 읽는다. Esc/우클릭/F6 follow 복귀는 아직 제출하지 않은 선택을 취소한다.
일반 좌클릭·gameplay 이동·스킬과 이 Debug 명령은 분리한다.

호출 흐름은 `MainApp F1 -> CPlayerController -> IPlayerCommandSink -> Shared -> Server GameRoom`이다.
Controller가 좌표 의도를 제출하고 Server가 현재 session/world, 순서, 생존 상태, 유한 좌표,
navigation 높이·walkability와 collision을 검증한다. 정상 위치만 기존 teleport reset 경로로 commit하고
기존 snapshot으로 표시한다. 거절은 이전 player/action을 보존하고 typed result 이유를 돌려준다.
Client는 send 성공을 이동 완료로 표시하거나 Character Transform을 직접 바꾸지 않는다.

Shared request/result와 protocol version, ServerApp/RoomCommand/GameRoom 소비자, NetworkManager 및
NetworkPlayerCommandSink의 결과 큐, PlayerController의 한 번 선택·응답 상태를 같은 변경에 연결한다.
Release Server는 명시적으로 거절한다. 적어도 발탄·쿠크 Arena에서 실제 F1 consumer를 제공한다.

## 관련 변경 단위

함께 요청한 Stage 자동 접기는 별도 커밋으로 묶고
[Composition Resources Stage 구현 계획](2026-09-05_COMPOSITION_RESOURCE_STAGE_ACCORDION_IMPLEMENTATION_PLAN.md)을 따른다.

## 검증과 완료 경계

- 기존 파일의 인코딩과 필요한 project/filter 등록을 확인한다. 새 C++ 파일과 새 JSON은 예정하지 않는다.
- 변경한 Client/Shared/Server의 최소 컴파일과 해당 protocol·Server 검사를 수행한다.
- 입력의 UI 차단·한 번 제출·취소, stale result, Server의 실패 시 기존 상태 보존을 확인한다.
- `git diff --check`와 변경 XML/JSON이 있을 경우 parse를 수행한다.
- 실제 F1 속도·F6 피킹·Stage 펼침은 사용자가 새 Server/Client에서 직접 확인한다.
  에이전트는 Client/UI 실행, 캡처 또는 visual PASS 판정을 하지 않는다.

## G02. 2026-09-06 Character Select·KoukuSaydon 플레이어 시점 저장

현재 `GB/koukusaydon-pattern-1-complete`의 공유 dirty 변경을 보존하고 사용자 요청의 카메라
설정만 추가한다. 두 Level은 `(0.4,7.5,4.5)` 위치 오프셋을 사용하고 Character Select의
주시점 높이/FOV/응답은 `1.05/45/18`, Kouku는 `1.2/60/0`이다. Camera_Free는 매 follow
갱신마다 LookAt을 다시 계산하므로 Transform 회전만 바꾸면 유지되지 않는다.

### G02-1. ArenaCameraProfile H/CPP와 JSON

새 `Client/Public/ArenaCameraProfile.h`, `Client/Private/ArenaCameraProfile.cpp`는
CHARACTER_SELECT와 KOUKU_SAYDON 두 typed map의 플레이어 상대 positionOffset(m),
rotationDegrees(Pitch/Yaw/Roll), focusDistance(m), FOV(deg), followResponse를 검증·저장한다.
Pitch 양수는 아래, yaw 0은 +Z이며 roll은 시선축 회전이다. lookOffset은 positionOffset에
회전한 +Z 방향과 focusDistance를 곱해 더한다. 기존 시점을 정확히 기본값으로 보존한다.
정본은 `Data/Camera/CharacterSelect.camera.json`, `Data/Camera/KoukuSaydon.camera.json`이며
schema/version/areaId와 finite 범위를 검증한다. Load 실패는 호출자가 가진 값을 보존하고 원인을
표시한다. Save는 검증한 전체 문서를 임시 파일에 기록한 뒤 원자 교체한다. 매 프레임 읽거나
Valtan 파일을 함께 저장하지 않는다. Client project의 ClInclude/ClCompile과 filters, 두 JSON의
96.DataFiles None 등록을 같은 변경에 추가한다. 별도 publisher와 하네스 파일은 만들지 않는다.

### G02-2. Camera_Free와 Level 연결

Camera_Free의 새 Set_FollowPose는 position/look offset, roll, FOV와 response를 한 번에
검증·적용한다. 기존 호출자의 roll 기본값은 0이며 기존 Valtan 동작을 유지한다. 각 Level이
profile과 로드 상태를 소유하고 생성 시 한 번 읽는다. Character Select class 변경, Kouku
player bind에서도 저장한 값을 사용한다. Kouku shot의 진입·복귀 eye/look/FOV도 동일 profile에서
계산하며 연출 자체는 기존 override를 따른다. 연출 동안 roll은 기존 연출이 소유하고 gameplay
follow로 복귀하면 profile roll이 적용된다. Server와 player Transform 권위는 변경하지 않는다.

### G02-3. MainApp F1 호출자

`Arena Camera / Player`를 Character Select에서도 표시하고 `Move Player` 아래에 두 맵 선택,
Position offset XYZ, Rotation Pitch/Yaw/Roll, Focus distance, FOV, Follow response와
Apply to current map / Save / Reload / Reset draft를 제공한다. 선택한 맵이 현재 활성 맵일 때만
Apply가 실제 camera를 바꾸며 다른 맵은 저장한 뒤 진입 시 소비한다. Save는 선택한 파일만 쓴다.
원래 자유 카메라 속도·Move Player·F6 경로를 보존한다. Valtan Level과 cinematic JSON은 수정하지
않고 기존 전후 바이트를 대조한다. 입력 draft, runtime Apply, JSON Save 상태를 구분해 표시한다.

### G02-4. 검증

새 profile C++ parse/save/load와 실패 시 기존 값 보존을 저장소 밖의 작은 기존 방식 CLI로 확인한다.
Camera_Free·두 Level·MainApp과 새 문서의 최소 컴파일, 새 JSON/project/filter parse,
`git diff --check`, Valtan 바이트 불변을 확인한다. 실행 파일 잠금이 없으면 표준 Debug Product
빌드로 링크·배포한다. 실행 중인 Client/Server를 에이전트가 종료하거나 UI 조작하지 않는다.
사용자가 F1 값 변경→Apply→Save→재진입과 F6 복귀·Kouku 연출 뒤 시점을 직접 확인한다.

### G02-5. 신규 파일의 구현 정본

ARENA_CAMERA_MAP은 저장 대상 두 맵만 식별한다. ARENA_CAMERA_PROFILE은 플레이어 상대 위치,
월드 기준 Pitch/Yaw/Roll, 주시 거리, 수직 화각과 follow 응답을 소유한다. Default는 기존 시점을
복원하고 LookOffset은 같은 rotation을 실제 LookAt 입력으로 변환한다. Validate는 유한 범위를,
Load는 schema/version/areaId를 함께 검사해 마지막에 outProfile을 교체한다. Save는 선택 파일의
변경 여부를 확인하고 임시 파일 왕복 검증 후 원자 교체한다. 실패 원인은 status로 실제 F1과
Level 호출자에 전달한다. 아래 새 파일과 두 JSON이 생성·재진입·Apply·Save의 실제 소비 입력이다.

#### Client/Public/ArenaCameraProfile.h

```cpp
#pragma once

#include "Client_Defines.h"
#include "Engine_Defines.h"

#include <filesystem>
#include <string>

NS_BEGIN(Client)

enum class ARENA_CAMERA_MAP
{
	CHARACTER_SELECT,
	KOUKU_SAYDON
};

struct ARENA_CAMERA_PROFILE final
{
	float3_t positionOffset{};
	// Pitch/Yaw/Roll in degrees; positive pitch looks down, yaw zero faces +Z.
	float3_t rotationDegrees{};
	f32_t focusDistance = 1.f;
	f32_t fovYDegrees = 60.f;
	f32_t followResponse = 0.f;
};

class CArenaCameraProfile final
{
public:
	static ARENA_CAMERA_PROFILE Default(ARENA_CAMERA_MAP map);
	static bool_t Validate(const ARENA_CAMERA_PROFILE& profile, std::string& status);
	// Failed reads preserve the caller's profile; Save only replaces this map's file.
	static bool_t Load(ARENA_CAMERA_MAP map, ARENA_CAMERA_PROFILE& outProfile,
		std::string& status);
	static bool_t Save(ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile,
		std::string& status);
	static float3_t LookOffset(const ARENA_CAMERA_PROFILE& profile);
	static std::filesystem::path Path(ARENA_CAMERA_MAP map);
};

NS_END
```

#### Client/Private/ArenaCameraProfile.cpp

```cpp
#include "ArenaCameraProfile.h"

#include "DataJson.h"
#include "ProjectDataRoot.h"

#include <atomic>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <limits>
#include <locale>
#include <sstream>
#include <stdexcept>

namespace
{
	constexpr const char* SCHEMA = "lostark.arena-camera";
	constexpr uint32_t FORMAT_VERSION = 1u;

	const char* AreaId(const ARENA_CAMERA_MAP map)
	{
		switch (map)
		{
		case ARENA_CAMERA_MAP::CHARACTER_SELECT: return "LV_LOBBY_CLASSSELECT_SL00";
		case ARENA_CAMERA_MAP::KOUKU_SAYDON: return "LV_LUT_MIDNIGHTC_ED";
		default: return nullptr;
		}
	}

	bool_t InRange(const f32_t value, const f32_t minimum, const f32_t maximum)
	{
		return std::isfinite(value) && value >= minimum && value <= maximum;
	}

	bool_t ReadFloat(const DATA_JSON_VALUE* value, f32_t& out)
	{
		if (nullptr == value || !value->Is_Number() ||
			!std::isfinite(value->Get_Number()) ||
			std::abs(value->Get_Number()) > (std::numeric_limits<f32_t>::max)())
			return false;
		out = static_cast<f32_t>(value->Get_Number());
		return true;
	}

	bool_t ReadVector(const DATA_JSON_VALUE* value, float3_t& out)
	{
		if (nullptr == value || !value->Is_Array() || value->Get_Array().size() != 3u)
			return false;
		const auto& values = value->Get_Array();
		return ReadFloat(&values[0], out.x) && ReadFloat(&values[1], out.y) &&
			ReadFloat(&values[2], out.z);
	}

	bool_t Parse(const std::string& text, const ARENA_CAMERA_MAP map,
		ARENA_CAMERA_PROFILE& out, std::string& status)
	{
		DATA_JSON_VALUE root;
		DATA_JSON_PARSE_LIMITS limits;
		limits.iMaximumBytes = 8192u;
		limits.iMaximumDepth = 4u;
		limits.iMaximumValues = 32u;
		if (!CDataJson::Parse(text, root, status, limits))
			return false;
		const char* area = AreaId(map);
		if (nullptr == area || !root.Is_Object() || root.Get_Object().size() != 8u)
		{
			status = "Arena camera document must contain exactly the eight supported fields.";
			return false;
		}
		const auto* schema = root.Find("schema");
		const auto* version = root.Find("formatVersion");
		const auto* areaId = root.Find("areaId");
		if (nullptr == schema || !schema->Is_String() || schema->Get_String() != SCHEMA ||
			nullptr == version || !version->Is_Number() || version->Was_FloatingPointToken() ||
			version->Get_Number() != FORMAT_VERSION || nullptr == areaId ||
			!areaId->Is_String() || areaId->Get_String() != area)
		{
			status = "Arena camera schema, integer formatVersion, or selected areaId is invalid.";
			return false;
		}
		ARENA_CAMERA_PROFILE staged;
		if (!ReadVector(root.Find("positionOffset"), staged.positionOffset) ||
			!ReadVector(root.Find("rotationDegrees"), staged.rotationDegrees) ||
			!ReadFloat(root.Find("focusDistance"), staged.focusDistance) ||
			!ReadFloat(root.Find("fovYDegrees"), staged.fovYDegrees) ||
			!ReadFloat(root.Find("followResponse"), staged.followResponse))
		{
			status = "Arena camera pose and lens fields must contain finite numbers.";
			return false;
		}
		if (!CArenaCameraProfile::Validate(staged, status))
			return false;
		out = staged;
		return true;
	}

	bool_t ReadText(const std::filesystem::path& path, std::string& out, std::string& status)
	{
		std::ifstream input(path, std::ios::binary | std::ios::ate);
		if (!input.is_open())
		{
			status = "Cannot read arena camera profile: " + path.string();
			return false;
		}
		const auto size = input.tellg();
		if (size < 0 || size > 8192)
		{
			status = "Arena camera profile exceeds the 8192-byte document limit: " + path.string();
			return false;
		}
		input.seekg(0);
		std::ostringstream buffer;
		buffer << input.rdbuf();
		if (input.bad())
		{
			status = "Arena camera profile read failed: " + path.string();
			return false;
		}
		out = buffer.str();
		return true;
	}

	std::string Serialize(const ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile)
	{
		std::ostringstream output;
		output.imbue(std::locale::classic());
		output << std::setprecision(std::numeric_limits<f32_t>::max_digits10)
			<< "{\n  \"schema\": \"" << SCHEMA << "\",\n  \"formatVersion\": " << FORMAT_VERSION
			<< ",\n  \"areaId\": \"" << AreaId(map) << "\",\n  \"positionOffset\": ["
			<< profile.positionOffset.x << ", " << profile.positionOffset.y << ", "
			<< profile.positionOffset.z << "],\n  \"rotationDegrees\": ["
			<< profile.rotationDegrees.x << ", " << profile.rotationDegrees.y << ", "
			<< profile.rotationDegrees.z << "],\n  \"focusDistance\": " << profile.focusDistance
			<< ",\n  \"fovYDegrees\": " << profile.fovYDegrees
			<< ",\n  \"followResponse\": " << profile.followResponse << "\n}\n";
		return output.str();
	}
}

std::filesystem::path CArenaCameraProfile::Path(const ARENA_CAMERA_MAP map)
{
	switch (map)
	{
	case ARENA_CAMERA_MAP::CHARACTER_SELECT:
		return CProjectDataRoot::Resolve(L"Camera/CharacterSelect.camera.json");
	case ARENA_CAMERA_MAP::KOUKU_SAYDON:
		return CProjectDataRoot::Resolve(L"Camera/KoukuSaydon.camera.json");
	default: return {};
	}
}

ARENA_CAMERA_PROFILE CArenaCameraProfile::Default(const ARENA_CAMERA_MAP map)
{
	if (nullptr == AreaId(map))
		throw std::invalid_argument("Unknown arena camera map.");
	const bool_t characterSelect = ARENA_CAMERA_MAP::CHARACTER_SELECT == map;
	ARENA_CAMERA_PROFILE profile;
	profile.positionOffset = { 0.4f, 7.5f, 4.5f };
	const f32_t deltaY = (characterSelect ? 1.05f : 1.2f) - profile.positionOffset.y;
	const f32_t horizontalDistance = std::hypot(profile.positionOffset.x, profile.positionOffset.z);
	profile.rotationDegrees = {
		XMConvertToDegrees(std::atan2(-deltaY, horizontalDistance)),
		XMConvertToDegrees(std::atan2(-profile.positionOffset.x, -profile.positionOffset.z)), 0.f
	};
	profile.focusDistance = std::hypot(deltaY, horizontalDistance);
	profile.fovYDegrees = characterSelect ? 45.f : 60.f;
	profile.followResponse = characterSelect ? 18.f : 0.f;
	return profile;
}

bool_t CArenaCameraProfile::Validate(const ARENA_CAMERA_PROFILE& profile, std::string& status)
{
	if (!InRange(profile.positionOffset.x, -1000.f, 1000.f) ||
		!InRange(profile.positionOffset.y, -1000.f, 1000.f) ||
		!InRange(profile.positionOffset.z, -1000.f, 1000.f))
		status = "Position offset components must be finite and within -1000..1000 metres.";
	else if (!InRange(profile.rotationDegrees.x, -89.f, 89.f) ||
		!InRange(profile.rotationDegrees.y, -180.f, 180.f) ||
		!InRange(profile.rotationDegrees.z, -180.f, 180.f))
		status = "Rotation requires pitch -89..89 and yaw/roll -180..180 degrees.";
	else if (!InRange(profile.focusDistance, 0.1f, 1000.f))
		status = "Focus distance must be finite and within 0.1..1000 metres.";
	else if (!InRange(profile.fovYDegrees, 10.f, 150.f))
		status = "Vertical FOV must be finite and within 10..150 degrees.";
	else if (!InRange(profile.followResponse, 0.f, 60.f))
		status = "Follow response must be finite and within 0..60 (0 is immediate).";
	else
	{
		status.clear();
		return true;
	}
	return false;
}

float3_t CArenaCameraProfile::LookOffset(const ARENA_CAMERA_PROFILE& profile)
{
	const auto rotation = XMMatrixRotationRollPitchYaw(
		XMConvertToRadians(profile.rotationDegrees.x),
		XMConvertToRadians(profile.rotationDegrees.y),
		XMConvertToRadians(profile.rotationDegrees.z));
	const auto forward = XMVector3TransformNormal(XMVectorSet(0.f, 0.f, 1.f, 0.f), rotation);
	float3_t look;
	XMStoreFloat3(&look, XMVectorMultiplyAdd(forward,
		XMVectorReplicate(profile.focusDistance), XMLoadFloat3(&profile.positionOffset)));
	return look;
}

bool_t CArenaCameraProfile::Load(const ARENA_CAMERA_MAP map, ARENA_CAMERA_PROFILE& outProfile,
	std::string& status)
{
	const auto path = Path(map);
	if (path.empty())
	{
		status = "Unknown arena camera map or unresolved project Data root.";
		return false;
	}
	std::string text;
	ARENA_CAMERA_PROFILE staged;
	if (!ReadText(path, text, status) || !Parse(text, map, staged, status))
		return false;
	outProfile = staged;
	status = "Loaded arena camera profile: " + path.string();
	return true;
}

bool_t CArenaCameraProfile::Save(const ARENA_CAMERA_MAP map, const ARENA_CAMERA_PROFILE& profile,
	std::string& status)
{
	const auto path = Path(map);
	if (path.empty())
	{
		status = "Unknown arena camera map or unresolved project Data root.";
		return false;
	}
	if (!Validate(profile, status))
		return false;
	std::error_code error;
	const bool_t existed = std::filesystem::exists(path, error);
	std::string previous;
	if (error || (existed && !ReadText(path, previous, status)))
	{
		if (error) status = "Cannot inspect arena camera destination: " + error.message();
		return false;
	}
	std::filesystem::create_directories(path.parent_path(), error);
	if (error)
	{
		status = "Cannot create arena camera directory: " + error.message();
		return false;
	}
	static std::atomic_uint64_t serial{ 0u };
	auto temporary = path;
	temporary += L".tmp." + std::to_wstring(GetCurrentProcessId()) + L"." +
		std::to_wstring(GetTickCount64()) + L"." + std::to_wstring(++serial);
	const HANDLE file = CreateFileW(temporary.c_str(), GENERIC_WRITE, 0, nullptr,
		CREATE_NEW, FILE_ATTRIBUTE_NORMAL, nullptr);
	if (INVALID_HANDLE_VALUE == file)
	{
		status = "Cannot create arena camera temporary file (Win32 " +
			std::to_string(GetLastError()) + ").";
		return false;
	}
	const std::string text = Serialize(map, profile);
	DWORD written = 0u;
	const bool_t writtenOk = FALSE != WriteFile(file, text.data(),
		static_cast<DWORD>(text.size()), &written, nullptr) && written == text.size() &&
		FALSE != FlushFileBuffers(file);
	CloseHandle(file);
	auto reject = [&](const std::string& reason)
	{
		std::error_code removeError;
		std::filesystem::remove(temporary, removeError);
		status = reason;
		return false;
	};
	if (!writtenOk)
		return reject("Arena camera temporary write failed; previous file preserved.");
	std::string stagedText;
	ARENA_CAMERA_PROFILE staged;
	if (!ReadText(temporary, stagedText, status) || !Parse(stagedText, map, staged, status))
		return reject("Arena camera saved-file validation failed: " + status);
	std::string current;
	const bool_t stillExists = std::filesystem::exists(path, error);
	if (error || stillExists != existed ||
		(stillExists && (!ReadText(path, current, status) || current != previous)))
		return reject("Arena camera file changed during Save; current file and draft preserved.");
	if (FALSE == MoveFileExW(temporary.c_str(), path.c_str(),
		(existed ? MOVEFILE_REPLACE_EXISTING : 0u) | MOVEFILE_WRITE_THROUGH))
		return reject("Arena camera atomic replace failed (Win32 " +
			std::to_string(GetLastError()) + "); previous file preserved.");
	status = "Saved arena camera profile: " + path.string();
	return true;
}
```

#### Data/Camera/CharacterSelect.camera.json

```json
{
  "schema": "lostark.arena-camera",
  "formatVersion": 1,
  "areaId": "LV_LOBBY_CLASSSELECT_SL00",
  "positionOffset": [0.400000006, 7.5, 4.5],
  "rotationDegrees": [54.9916306, -174.92038, 0],
  "focusDistance": 7.87480116,
  "fovYDegrees": 45,
  "followResponse": 18
}
```

#### Data/Camera/KoukuSaydon.camera.json

```json
{
  "schema": "lostark.arena-camera",
  "formatVersion": 1,
  "areaId": "LV_LUT_MIDNIGHTC_ED",
  "positionOffset": [0.400000006, 7.5, 4.5],
  "rotationDegrees": [54.3556175, -174.92038, 0],
  "focusDistance": 7.75241899,
  "fovYDegrees": 60,
  "followResponse": 0
}
```
