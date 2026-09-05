# 2026-09-04 쿠크세이튼 팝업북 카메라 연출과 1Stage 진입 트리거 PLAN

이 문서는 두 작업의 반영 코드 정본이다.

1. 책이 펼쳐지는 컷신에 실제로 움직이는 카메라 연출을 붙인다.
2. 1Stage 끝에서 그 컷신이 돌고, 끝난 뒤 캐릭터가 아레나에 등장하는 트리거를 설치한다.

---

## 0. 지금 코드와 데이터 실측

작성 전에 확인한 사실만 적는다. 파일과 줄은 현재 저장소 기준이다.

### 0.1 컷신 재생 사슬 (제품 레벨)

```text
Server CServerTriggerSystem::Evaluate_Entries        Server/Private/ServerTriggerSystem.cpp:145
  -> WORLD_TRIGGER_ACTION_KIND::PLAY_SEQUENCE 이면 activateTarget 콜백
  -> CGameRoom 람다가 Broadcast_WorldSequencePlay(targetId)   Server/Private/GameRoom.cpp:1889
  -> S2C_WORLD_SEQUENCE_PLAY                                   Shared/Public/Network/PacketMessages.h:1824
Client CClientReplication 이벤트 큐                            Client/Private/ClientReplication.cpp:352
  -> CLevel_KakulSaydonArena::Update 에서 Consume_WorldSequencePlays()  Level_KakulSaydonArena.cpp:467
  -> m_SequencePlayer.Play(instanceId, targets)                          Level_KakulSaydonArena.cpp:499
```

시퀀스 재생은 Client 표현이고 트리거 판정은 Server authority다. 이 경계는 유지한다.

### 0.2 카메라 샷 (지금 있는 것)

- 문서: `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json`,
  schema `lostark.camera-shots`, formatVersion 1.
- 구조체 `KAKUL_CAMERA_SHOT`: `Client/Public/Level_KakulSaydonArena.h:42`.
  eye/lookAt/fov 각각 하나뿐인 **고정 포즈**다.
- 파서: `Level_KakulSaydonArena.cpp:877`의 `Has_ExactProperties` 목록이
  `shotId, sequenceInstanceId, box, eye, lookAt, fovYDegrees, blendInMs, blendOutMs, priority`
  아홉 개를 정확히 요구한다. 여기에 없는 키가 있으면 문서 전체를 거부한다.
- 재생: `Update_CameraShots` (`Level_KakulSaydonArena.cpp:1006`)가 활성 샷의 고정 포즈를
  `m_vCameraEyeTo/m_vCameraLookTo/m_fCameraFovTo`에 넣고 smoothstep으로 블렌드한다.
- 맵툴: `Render_CameraShotSection` (`MapTool.cpp:13059`)이 저작하고
  `Save_CameraShots` (`MapTool.cpp:12933`)가 문자열로 직접 직렬화한다.

고정 포즈뿐이므로 지금 구조로는 "카메라 연출"을 만들 수 없다.

### 0.3 이미 있는 시네마틱 카메라 샘플러

`CValtanCinematicCameraController` (`Client/Public/ValtanCinematicCameraController.h`)에
키프레임 샘플러가 이미 있다.

```text
static bool_t Sample_Cue(const VALTAN_CINEMATIC_CAMERA_CUE&, f32_t elapsedSeconds,
                         VALTAN_CINEMATIC_CAMERA_POSE&)
static bool_t Sample_BoundedTransition(from, to, durationMs, elapsedSeconds, out)
```

`VALTAN_CINEMATIC_CAMERA_CUE`는 `iDurationMs`, `eInterpolation`(LINEAR/CATMULL_ROM),
`eEasing`(LINEAR/SMOOTHSTEP/HOLD), `eTrackingMode`, `Keyframes`를 가진다
(`Client/Public/ValtanCinematicCameraDocument.h:63`).

같은 역할의 두 번째 보간 구현을 만들지 않기 위해, 쿠크 카메라도 이 샘플러를 그대로 쓴다.
문서만 쿠크가 소유하고, 파싱 결과를 `VALTAN_CINEMATIC_CAMERA_CUE` 하나로 만들어 넘긴다.
`eTrackingMode`는 `WORLD` 고정이다. 쿠크 컷신에는 추적할 보스가 없다.

### 0.4 컷신 시계

`CWorldSequencePlayer`는 `ACTIVE_INSTANCE.elapsedMs`를 내부에만 두고
`Is_Playing(instanceId)`만 공개한다 (`Client/Public/WorldSequencePlayer.h:73`).
카메라가 컷신 시각을 알 방법이 지금은 없다.

### 0.5 1Stage 데이터

`Data/Worlds/LV_LUT_MIDNIGHTC_ED/Gameplay.world.json` formatVersion 6.

```text
triggerBox 1Stage_Final  pos (27.8, 0.45, -67.7) yaw 123.5 half (6,1,1) triggerOnce
  events: [ { type: playSequence, sequenceInstanceId: world.sequence.instance.circusfinale } ]
playerSpawn stage.kakul.sl04  pos (-2, 1.299, 730)     <- 아레나
playerSpawn player.spawn.kakul.party01..04  pos (3.3, 8.64, -10.69)  <- 1Stage 입구
```

컷신 인스턴스는 `world.sequence.instance.original_8T6_00..04`와 `original_book`
다섯 + 한 개다. 재생 길이는 4507ms를 `playbackSpeed 0.7`로 돌려 6439ms다.

### 0.6 트리거의 현재 제약

- publisher `Tools/WorldPipeline/Publish-WorldGameplay.ps1:546`이
  `events.Count -ne 1`이면 거부한다. 즉 지금은 트리거 하나에 이벤트 하나뿐이다.
- Server `WorldBootstrap.cpp:334~395`는 이미 **행 하나에 액션 여러 개**를 읽어
  `placement.TriggerActions`에 push한다. 서버는 이미 다중 액션을 지원한다.
- `WORLD_TRIGGER_ACTION` (`Server/Public/WorldBootstrap.h:97`)에는 지연 필드가 없다.
  모든 액션이 진입한 tick에 즉시 실행된다.

즉 "컷신을 틀고, 끝난 뒤 플레이어를 옮긴다"를 표현할 수단이 지금은 없다.

---

## G01. 컷신 시계 공개

### 파일 역할과 위치

- `Client/Public/WorldSequencePlayer.h` — 시퀀스 재생기의 public 계약.
- `Client/Private/WorldSequencePlayer.cpp` — 구현.

카메라는 컷신의 경과 시간을 알아야 한다. 재생기만 그 값을 소유하므로 재생기가 공개한다.
새 상태를 만들지 않고 이미 있는 `ACTIVE_INSTANCE::elapsedMs`를 읽기 전용으로 내보낸다.

### 선언

`Is_Playing` 선언 바로 아래에 추가한다.

```cpp
	/* The camera cue runs on the cutscene's own clock. Only the player owns
	   that clock, so it hands out a read-only sample instead of letting a
	   second owner count the same time. false means the instance is not
	   playing and the caller must not pose a camera from a stale value. */
	bool_t Try_GetElapsedMs(
		const std::string& instanceId,
		f32_t& outElapsedMs) const;
```

### 함수 한 줄 책임과 흐름

`Try_GetElapsedMs` — 재생 중인 인스턴스의 경과 밀리초를 그대로 돌려준다.

```text
호출자: CLevel_KakulSaydonArena::Update_CameraShots, CMapTool 미리보기
→ m_Active에서 instanceId 검색
→ 없으면 false, 출력값 손대지 않음
→ 있으면 elapsedMs 복사 후 true
→ 상태 변경 없음
```

### 전체 코드

`Client/Private/WorldSequencePlayer.cpp`의 `Is_Playing` 정의 바로 아래에 넣는다.

```cpp
bool_t CWorldSequencePlayer::Try_GetElapsedMs(
	const std::string& instanceId,
	f32_t& outElapsedMs) const
{
	const auto found = std::find_if(m_Active.begin(), m_Active.end(),
		[&instanceId](const ACTIVE_INSTANCE& value)
		{
			return value.instanceId == instanceId;
		});
	if (m_Active.end() == found)
		return false;
	outElapsedMs = found->elapsedMs;
	return true;
}
```

### 검증

- Client Debug 빌드.
- 이 G만으로는 화면 변화가 없다. G03의 소비자가 붙어야 완료다.

---

## G02. 카메라 샷에 키프레임 저장

### 파일 역할과 위치

- `Data/Maps/Authoring/LV_LUT_MIDNIGHTC_ED/LV_LUT_MIDNIGHTC_ED.camerashots.json` — 저작 정본.
- `Client/Bin/DataFiles/Map/LV_LUT_MIDNIGHTC_ED.camerashots.json` — publisher 생성물.
- `Client/Public/Level_KakulSaydonArena.h` — 런타임 구조체.
- `Client/Private/Level_KakulSaydonArena.cpp` — 파서.

기존 고정 포즈 샷을 그대로 두고, 같은 문서의 샷에 **선택적** 키프레임 배열을 더한다.
키프레임이 없으면 지금과 완전히 같게 동작한다. 새 문서를 만들지 않는 이유는
맵툴 저작 흐름과 제품 로더가 이미 이 문서 하나를 정본으로 쓰고 있기 때문이다.

### 데이터 형식

`formatVersion`은 1에서 2로 올린다. 키프레임은 선택 항목이지만 형식이 늘어났기 때문이다.

```json
{
 "shotId": "shot.original",
 "sequenceInstanceId": "world.sequence.instance.original_8T6_00",
 "box": { "center": [0.0, 8.0, 737.28], "halfExtents": [60.0, 50.0, 60.0], "yawDegrees": 0 },
 "eye": [15.849, 0.539, 727.685],
 "lookAt": [2.886, -0.032, 741.052],
 "fovYDegrees": 50,
 "blendInMs": 800,
 "blendOutMs": 1200,
 "priority": 10,
 "cameraTrack": {
  "durationMs": 6439,
  "interpolation": "CATMULL_ROM",
  "easing": "SMOOTHSTEP",
  "keyframes": [
   { "sceneId": "shot.original.k00", "timeMs": 0,    "eye": [15.849, 0.539, 727.685], "lookAt": [2.886, -0.032, 741.052], "fovYDegrees": 50 },
   { "sceneId": "shot.original.k01", "timeMs": 6439, "eye": [15.849, 0.539, 727.685], "lookAt": [2.886, -0.032, 741.052], "fovYDegrees": 50 }
  ]
 }
}
```

불변식.

- `cameraTrack`이 없으면 기존 고정 포즈 샷이다.
- `cameraTrack`이 있으면 `keyframes`는 2개 이상, `timeMs`는 0에서 시작해 순증가하고
  마지막 값이 `durationMs`와 같다.
- `eye`와 `lookAt`은 같은 키프레임에서 같은 점이면 안 된다. 시선 벡터가 0이 된다.
- `sceneId`는 문서 안에서 유일한 stable ID다. 배열 index를 ID로 쓰지 않는다.

### 선언

`Client/Public/Level_KakulSaydonArena.h`의 `KAKUL_CAMERA_SHOT` 구조체(현재 42~58줄)를
아래 블록으로 통째로 교체한다. `#include "ValtanCinematicCameraDocument.h"`를 이 헤더의
include 목록에 추가한다. 큐 타입을 값으로 들고 있어야 하므로 전방 선언으로는 부족하다.

```cpp
	struct KAKUL_CAMERA_SHOT final
	{
		std::string strShotId;
		/* Empty means the box decides. When it names a sequence the
		   shot holds for exactly as long as that sequence plays, so a
		   trigger that starts the sequence also starts the shot. */
		std::string strSequenceInstanceId;
		float3_t vCenter = {};
		float3_t vHalfExtents = {};
		f32_t fYawDegrees = 0.f;
		float3_t vEye = {};
		float3_t vLookAt = {};
		f32_t fFovYDegrees = 60.f;
		uint32_t iBlendInMs = 0u;
		uint32_t iBlendOutMs = 0u;
		uint32_t iPriority = 0u;
		/* A shot without a track keeps the single authored pose. With one it
		   is sampled on the bound sequence's own clock by the one cinematic
		   sampler the project already owns, so no second easing or spline
		   implementation can drift from it. */
		bool_t hasCameraTrack = false;
		VALTAN_CINEMATIC_CAMERA_CUE CameraTrack;
	};
```

### 파서 교체 블록

`Client/Private/Level_KakulSaydonArena.cpp`의 `Has_ExactProperties` 호출(현재 882~887줄)을
아래로 교체한다. 나머지 파싱 코드는 그대로 둔다.

```cpp
		if (!Has_ExactProperties(value,
			{ "shotId", "sequenceInstanceId", "box", "eye", "lookAt",
				"fovYDegrees", "blendInMs", "blendOutMs", "priority" }) &&
			!Has_ExactProperties(value,
				{ "shotId", "sequenceInstanceId", "box", "eye", "lookAt",
					"fovYDegrees", "blendInMs", "blendOutMs", "priority",
					"cameraTrack" }))
		{
			outStatus = "KoukuSaton camera shot has unexpected properties.";
			return false;
		}
```

그리고 같은 루프에서 `shot.fFovYDegrees = ...` 다음, `stagedShots.push_back` 앞에
아래 블록을 넣는다.

```cpp
		const DATA_JSON_VALUE* cameraTrack = value.Find("cameraTrack");
		if (nullptr != cameraTrack)
		{
			if (!Read_CameraTrack(*cameraTrack, shot.strShotId,
				shot.CameraTrack, outStatus))
			{
				return false;
			}
			shot.hasCameraTrack = true;
		}
```

### 새 파서 함수

같은 파일의 익명 namespace 안, `Contains_CameraShot` 정의 바로 위에 넣는다.
헤더 선언은 필요 없다. 이 파일 안에서만 쓰인다.

```cpp
	bool Read_CameraTrack(
		const Client::DATA_JSON_VALUE& value,
		const std::string& shotId,
		Client::VALTAN_CINEMATIC_CAMERA_CUE& outCue,
		std::string& outStatus)
	{
		if (DATA_JSON_TYPE::OBJECT != value.Get_Type() ||
			!Has_ExactProperties(value,
				{ "durationMs", "interpolation", "easing", "keyframes" }))
		{
			outStatus = "KoukuSaton camera track shape is invalid: " + shotId;
			return false;
		}
		const DATA_JSON_VALUE* duration =
			Required(value, "durationMs", DATA_JSON_TYPE::NUMBER);
		const DATA_JSON_VALUE* interpolation =
			Required(value, "interpolation", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* easing =
			Required(value, "easing", DATA_JSON_TYPE::STRING);
		const DATA_JSON_VALUE* keyframes =
			Required(value, "keyframes", DATA_JSON_TYPE::ARRAY);
		uint32_t durationMs = 0u;
		if (nullptr == duration || nullptr == interpolation ||
			nullptr == easing || nullptr == keyframes ||
			!Read_Uint(duration, CAMERA_TRACK_MAX_DURATION_MS, durationMs) ||
			0u == durationMs ||
			keyframes->Get_Array().size() < 2u ||
			keyframes->Get_Array().size() > CAMERA_TRACK_MAX_KEYFRAMES)
		{
			outStatus = "KoukuSaton camera track values are invalid: " + shotId;
			return false;
		}
		if ("LINEAR" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR;
		}
		else if ("CATMULL_ROM" == interpolation->Get_String())
		{
			outCue.eInterpolation =
				Client::VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
		}
		else
		{
			outStatus = "KoukuSaton camera track interpolation is unknown: " + shotId;
			return false;
		}
		if ("LINEAR" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::LINEAR;
		else if ("SMOOTHSTEP" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP;
		else if ("HOLD" == easing->Get_String())
			outCue.eEasing = Client::VALTAN_CINEMATIC_CAMERA_EASING::HOLD;
		else
		{
			outStatus = "KoukuSaton camera track easing is unknown: " + shotId;
			return false;
		}
		outCue.strCueId = shotId;
		outCue.iDurationMs = durationMs;
		outCue.iTransitionInMs = 0u;
		outCue.iTransitionOutMs = 0u;
		outCue.eTrackingMode = Client::VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
		outCue.vTrackingOrigin = float3_t(0.f, 0.f, 0.f);
		outCue.fShakeAmplitude = 0.f;
		outCue.iShakeDurationMs = 0u;
		outCue.Keyframes.clear();
		outCue.Keyframes.reserve(keyframes->Get_Array().size());
		uint32_t previousTimeMs = 0u;
		std::unordered_set<std::string> sceneIds;
		for (const DATA_JSON_VALUE& entry : keyframes->Get_Array())
		{
			if (!Has_ExactProperties(entry,
				{ "sceneId", "timeMs", "eye", "lookAt", "fovYDegrees" }))
			{
				outStatus = "KoukuSaton camera keyframe shape is invalid: " + shotId;
				return false;
			}
			Client::VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe;
			const DATA_JSON_VALUE* sceneId =
				Required(entry, "sceneId", DATA_JSON_TYPE::STRING);
			const DATA_JSON_VALUE* fov =
				Required(entry, "fovYDegrees", DATA_JSON_TYPE::NUMBER);
			uint32_t timeMs = 0u;
			if (nullptr == sceneId || !Is_StableId(sceneId->Get_String()) ||
				!sceneIds.emplace(sceneId->Get_String()).second ||
				!Read_Uint(entry.Find("timeMs"), durationMs, timeMs) ||
				!Read_Float3(entry.Find("eye"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vEye) ||
				!Read_Float3(entry.Find("lookAt"), CAMERA_SHOT_MAX_COORDINATE,
					keyframe.vLookAt) ||
				nullptr == fov || !std::isfinite(fov->Get_Number()) ||
				fov->Get_Number() <= 1.0 || fov->Get_Number() >= 179.0)
			{
				outStatus = "KoukuSaton camera keyframe values are invalid: " + shotId;
				return false;
			}
			if (outCue.Keyframes.empty())
			{
				if (0u != timeMs)
				{
					outStatus = "KoukuSaton camera track must start at 0ms: " + shotId;
					return false;
				}
			}
			else if (timeMs <= previousTimeMs)
			{
				outStatus = "KoukuSaton camera keyframes must advance: " + shotId;
				return false;
			}
			const float3_t direction(
				keyframe.vLookAt.x - keyframe.vEye.x,
				keyframe.vLookAt.y - keyframe.vEye.y,
				keyframe.vLookAt.z - keyframe.vEye.z);
			if (CAMERA_TRACK_MIN_LOOK_DISTANCE >
				std::sqrt(direction.x * direction.x +
					direction.y * direction.y + direction.z * direction.z))
			{
				outStatus = "KoukuSaton camera keyframe has no view direction: " + shotId;
				return false;
			}
			keyframe.strSceneId = sceneId->Get_String();
			keyframe.iTimeMs = timeMs;
			keyframe.fFovYDegrees = static_cast<f32_t>(fov->Get_Number());
			previousTimeMs = timeMs;
			outCue.Keyframes.push_back(std::move(keyframe));
		}
		if (outCue.Keyframes.back().iTimeMs != durationMs)
		{
			outStatus = "KoukuSaton camera track must end at its duration: " + shotId;
			return false;
		}
		return true;
	}
```

같은 익명 namespace의 상수 블록(현재 67~78줄, `CAMERA_SHOT_SCHEMA` 아래)에 추가한다.

```cpp
	constexpr uint32_t CAMERA_TRACK_MAX_DURATION_MS = 120000u;
	constexpr size_t CAMERA_TRACK_MAX_KEYFRAMES = 64u;
	constexpr f32_t CAMERA_TRACK_MIN_LOOK_DISTANCE = 0.01f;
```

### 검증

- 잘못된 문서 네 가지를 각각 넣어 로드가 거부되고 기존 샷이 보존되는지 본다.
  키프레임 1개, timeMs 역순, 마지막 timeMs != durationMs, eye == lookAt.
- 정상 문서에서 `Loaded N shot(s)` 상태와 샷 수가 맞는지 본다.

---

## G03. 제품 레벨에서 카메라 트랙 재생

### 파일 역할과 위치

`Client/Private/Level_KakulSaydonArena.cpp`의 `Update_CameraShots`가 유일한 소비자다.
`#include "ValtanCinematicCameraController.h"`를 이 파일에 추가한다.

### 함수 한 줄 책임

`Update_CameraShots` — 활성 샷의 목표 포즈를 정하고 블렌드해 카메라에 제출한다.
이번 G는 "목표 포즈를 정한다" 부분만 바뀐다. 블렌드와 소유권 처리는 그대로다.

### 흐름 (바뀌는 부분만)

```text
활성 샷 결정 (기존)
→ 샷에 cameraTrack이 있고 묶인 시퀀스가 재생 중이면
   m_SequencePlayer.Try_GetElapsedMs 로 경과 시각을 얻는다
→ CValtanCinematicCameraController::Sample_Cue(track, elapsed/1000, pose)
→ 성공하면 pose를 목표로 쓴다
→ 실패하거나 트랙이 없으면 기존 고정 포즈를 목표로 쓴다
→ 이후 블렌드/제출은 기존 그대로
```

시퀀스가 끝나면 `Is_Playing`이 false가 되어 `Find_ActiveCameraShot`이 그 샷을 고르지
않는다. 그때부터 기존 blendOut 경로가 팔로우 카메라로 되돌린다. 새 종료 처리를 만들지 않는다.

### 교체 블록

`Update_CameraShots` 안의 아래 블록(현재 1080~1085줄)을

```cpp
	if (nullptr != shot)
	{
		m_vCameraEyeTo = shot->vEye;
		m_vCameraLookTo = shot->vLookAt;
		m_fCameraFovTo = shot->fFovYDegrees;
	}
```

다음으로 교체한다.

```cpp
	if (nullptr != shot)
	{
		m_vCameraEyeTo = shot->vEye;
		m_vCameraLookTo = shot->vLookAt;
		m_fCameraFovTo = shot->fFovYDegrees;
		f32_t cueElapsedMs = 0.f;
		VALTAN_CINEMATIC_CAMERA_POSE cuePose{};
		if (shot->hasCameraTrack &&
			!shot->strSequenceInstanceId.empty() &&
			m_SequencePlayer.Try_GetElapsedMs(
				shot->strSequenceInstanceId, cueElapsedMs) &&
			CValtanCinematicCameraController::Sample_Cue(
				shot->CameraTrack, cueElapsedMs / 1000.f, cuePose))
		{
			/* The cue owns the framing for as long as the cutscene runs; the
			   authored single pose stays as the fallback so a rejected sample
			   never leaves the camera on a stale frame. */
			m_vCameraEyeTo = cuePose.vEye;
			m_vCameraLookTo = cuePose.vLookAt;
			m_fCameraFovTo = cuePose.fFovYDegrees;
		}
	}
```

### 검증

- 트랙이 없는 샷(`shot.1`)은 기존과 동일하게 동작한다.
- 트랙이 있는 샷은 컷신 재생 중 매 프레임 다른 포즈가 나온다. 시작 프레임의 포즈가
  키프레임 0과 같고, 끝 프레임이 마지막 키프레임과 같은지 확인한다.
- 컷신이 끝나면 blendOut 뒤 팔로우 카메라로 돌아온다.

---

## G04. 맵툴에서 카메라 트랙 저작과 미리보기

### 파일 역할과 위치

- `Client/Public/MapTool.h` — 에디터 구조체와 멤버.
- `Client/Private/MapTool.cpp` — 로드/저장/UI/미리보기.

맵툴은 제품과 같은 문서를 읽고 쓴다. 저작 결과를 사람이 눈으로 확인하지 못하면
이 작업은 완료가 아니므로, 컷신 재생 중 카메라를 실제로 따라가게 한다.

### 선언

`EDITOR_CAMERA_SHOT`(현재 `MapTool.h:66`) 아래에 키프레임 구조체를 추가하고
샷에 트랙 필드를 더한다.

```cpp
	struct EDITOR_CAMERA_KEYFRAME final
	{
		std::string sceneId;
		int32_t timeMs = 0;
		float3_t eye = {};
		float3_t lookAt = {};
		f32_t fovYDegrees = 50.f;
	};
```

`EDITOR_CAMERA_SHOT`의 `int32_t priority = 10;` 바로 아래에 추가한다.

```cpp
		/* Empty keeps the single authored pose. Non-empty is sampled on the
		   bound sequence clock exactly like the product level does. */
		std::vector<EDITOR_CAMERA_KEYFRAME> keyframes;
		int32_t trackDurationMs = 0;
		int32_t interpolationIndex = 1;   /* 0 LINEAR, 1 CATMULL_ROM */
		int32_t easingIndex = 1;          /* 0 LINEAR, 1 SMOOTHSTEP, 2 HOLD */
```

멤버 변수는 `bool_t m_bCameraShotPreviewActive = false;` 아래에 추가한다.

```cpp
	/* True while the cutscene preview holds the editor camera. */
	bool_t m_bCutsceneCameraHeld = false;
```

### 함수 한 줄 책임

- `Load_CameraShots` — 문서를 읽어 `m_CameraShots`를 채운다. 이번 G에서 `cameraTrack`을 읽는다.
- `Save_CameraShots` — `m_CameraShots`를 문서로 쓴다. 이번 G에서 `cameraTrack`을 쓴다.
- `Render_CameraShotSection` — 샷 편집 UI. 이번 G에서 키프레임 목록과 캡처 버튼을 더한다.
- `Apply_CutsceneCameraTrack` (신규) — 컷신이 도는 동안 트랙을 샘플해 에디터 카메라에 적용한다.
- `End_CutsceneCameraTrack` (신규) — 카메라 소유권을 돌려준다.

### 신규 함수 전체 코드

`Client/Private/MapTool.cpp`의 `Apply_CutsceneArenaVisibility` 정의 바로 위에 넣는다.

```cpp
void Client::CMapTool::Apply_CutsceneCameraTrack()
{
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr == camera)
		return;
	const EDITOR_CAMERA_SHOT* bound = nullptr;
	f32_t elapsedMs = 0.f;
	for (const EDITOR_CAMERA_SHOT& shot : m_CameraShots)
	{
		if (shot.keyframes.size() < 2u || shot.sequenceInstanceId.empty())
			continue;
		f32_t candidateMs = 0.f;
		if (!m_ArenaRisePlayer.Try_GetElapsedMs(
			shot.sequenceInstanceId, candidateMs))
		{
			continue;
		}
		if (nullptr == bound || shot.priority > bound->priority)
		{
			bound = &shot;
			elapsedMs = candidateMs;
		}
	}
	if (nullptr == bound)
	{
		End_CutsceneCameraTrack();
		return;
	}
	VALTAN_CINEMATIC_CAMERA_CUE cue{};
	cue.strCueId = bound->shotId;
	cue.iDurationMs = static_cast<uint32_t>((std::max)(1, bound->trackDurationMs));
	cue.eInterpolation = 0 == bound->interpolationIndex ?
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION::LINEAR :
		VALTAN_CINEMATIC_CAMERA_INTERPOLATION::CATMULL_ROM;
	cue.eEasing = 0 == bound->easingIndex ?
		VALTAN_CINEMATIC_CAMERA_EASING::LINEAR :
		(1 == bound->easingIndex ?
			VALTAN_CINEMATIC_CAMERA_EASING::SMOOTHSTEP :
			VALTAN_CINEMATIC_CAMERA_EASING::HOLD);
	cue.eTrackingMode = VALTAN_CINEMATIC_TRACKING_MODE::WORLD;
	cue.Keyframes.reserve(bound->keyframes.size());
	for (const EDITOR_CAMERA_KEYFRAME& source : bound->keyframes)
	{
		VALTAN_CINEMATIC_CAMERA_KEYFRAME keyframe{};
		keyframe.strSceneId = source.sceneId;
		keyframe.iTimeMs = static_cast<uint32_t>((std::max)(0, source.timeMs));
		keyframe.vEye = source.eye;
		keyframe.vLookAt = source.lookAt;
		keyframe.fFovYDegrees = source.fovYDegrees;
		cue.Keyframes.push_back(std::move(keyframe));
	}
	VALTAN_CINEMATIC_CAMERA_POSE pose{};
	if (!CValtanCinematicCameraController::Sample_Cue(
		cue, elapsedMs / 1000.f, pose))
	{
		End_CutsceneCameraTrack();
		return;
	}
	if (!m_bCutsceneCameraHeld)
	{
		if (!camera->Begin_PresentationOverride(
			CAMERA_SHOT_PREVIEW_OWNER_ID,
			CCamera::PRESENTATION_PRIORITY::AUTHORING_PREVIEW))
		{
			return;
		}
		m_bCutsceneCameraHeld = true;
	}
	if (!camera->Apply_PresentationPose(CAMERA_SHOT_PREVIEW_OWNER_ID,
		pose.vEye, pose.vLookAt, pose.fFovYDegrees))
	{
		End_CutsceneCameraTrack();
	}
}

void Client::CMapTool::End_CutsceneCameraTrack()
{
	if (!m_bCutsceneCameraHeld)
		return;
	const shared_ptr<CCamera_Free> camera = m_pAssetTestCamera.lock();
	if (nullptr != camera)
		camera->End_PresentationOverride(CAMERA_SHOT_PREVIEW_OWNER_ID);
	m_bCutsceneCameraHeld = false;
}
```

`Update_CutsceneArenaRise`의 인계 블록을 아래로 교체한다.

```cpp
	if (m_bCutsceneOriginalRunning && !Is_CutsceneOriginalPlaying())
	{
		m_bCutsceneOriginalRunning = false;
		End_CutsceneCameraTrack();
		Hide_CutsceneSet();
		Apply_CutsceneArenaVisibility(false);
		m_Status = "Original cutscene finished: arena handed back";
	}
	else if (m_bCutsceneOriginalRunning)
	{
		Apply_CutsceneCameraTrack();
	}
```

`Stop arena rise` 버튼의 정리 블록에도 `End_CutsceneCameraTrack();`을
`Hide_CutsceneSet();` 바로 위에 넣는다.

### UI 추가 블록

`Render_CameraShotSection`의 샷 편집부에서 `ImGui::DragInt("Priority", ...)` 바로 아래에
넣는다.

```cpp
			ImGui::Separator();
			ImGui::Text("Camera track keys: %zu", shot.keyframes.size());
			ImGui::DragInt("Track Duration (ms)", &shot.trackDurationMs,
				10.f, 0, 120000);
			ImGui::Combo("Interpolation", &shot.interpolationIndex,
				"LINEAR\0CATMULL_ROM\0\0");
			ImGui::Combo("Easing", &shot.easingIndex,
				"LINEAR\0SMOOTHSTEP\0HOLD\0\0");
			ImGui::BeginDisabled(!hasEditorPose);
			if (ImGui::Button("Append Key From Camera"))
			{
				EDITOR_CAMERA_KEYFRAME keyframe{};
				char sceneBuffer[160]{};
				(void)std::snprintf(sceneBuffer, sizeof(sceneBuffer), "%s.k%02zu",
					shot.shotId.c_str(), shot.keyframes.size());
				keyframe.sceneId = sceneBuffer;
				keyframe.timeMs = shot.keyframes.empty() ? 0 :
					shot.keyframes.back().timeMs + 1000;
				keyframe.eye = editorEye;
				keyframe.lookAt = editorLookAt;
				keyframe.fovYDegrees = shot.fovYDegrees;
				shot.trackDurationMs = (std::max)(shot.trackDurationMs,
					keyframe.timeMs);
				shot.keyframes.push_back(std::move(keyframe));
				m_CameraShotStatus = "Appended a camera key to " + shot.shotId;
			}
			ImGui::EndDisabled();
			ImGui::SameLine();
			if (ImGui::Button("Clear Keys"))
			{
				shot.keyframes.clear();
				shot.trackDurationMs = 0;
				m_CameraShotStatus = "Cleared the camera track of " + shot.shotId;
			}
			for (size_t keyIndex = 0; keyIndex < shot.keyframes.size(); ++keyIndex)
			{
				EDITOR_CAMERA_KEYFRAME& keyframe = shot.keyframes[keyIndex];
				ImGui::PushID(static_cast<int>(1000 + keyIndex));
				ImGui::DragInt("Key Time (ms)", &keyframe.timeMs, 10.f, 0, 120000);
				ImGui::DragFloat3("Key Eye", &keyframe.eye.x, 0.1f);
				ImGui::DragFloat3("Key Look At", &keyframe.lookAt.x, 0.1f);
				ImGui::DragFloat("Key Fov Y", &keyframe.fovYDegrees, 0.25f, 5.f, 170.f);
				ImGui::PopID();
			}
```

### 로드와 저장

`Load_CameraShots`는 `priority`를 읽은 다음 `cameraTrack`이 있으면 위 필드를 채운다.
`Save_CameraShots`는 `shot.keyframes.size() >= 2`일 때만 `cameraTrack` 객체를 쓴다.
두 함수 모두 현재 파일에서 문자열로 직렬화하므로, 저장 블록은 아래 형태를 따른다.

```cpp
		if (2u <= shot.keyframes.size())
		{
			text += ",\n      \"cameraTrack\": {\n";
			text += "        \"durationMs\": " +
				std::to_string(shot.trackDurationMs) + ",\n";
			text += "        \"interpolation\": \"" +
				std::string(0 == shot.interpolationIndex ?
					"LINEAR" : "CATMULL_ROM") + "\",\n";
			text += "        \"easing\": \"" +
				std::string(0 == shot.easingIndex ? "LINEAR" :
					(1 == shot.easingIndex ? "SMOOTHSTEP" : "HOLD")) + "\",\n";
			text += "        \"keyframes\": [\n";
			for (size_t keyIndex = 0; keyIndex < shot.keyframes.size(); ++keyIndex)
			{
				const EDITOR_CAMERA_KEYFRAME& keyframe = shot.keyframes[keyIndex];
				text += "          { \"sceneId\": \"" + keyframe.sceneId +
					"\", \"timeMs\": " + std::to_string(keyframe.timeMs) +
					", \"eye\": " + Format_Float3(keyframe.eye) +
					", \"lookAt\": " + Format_Float3(keyframe.lookAt) +
					", \"fovYDegrees\": " + Format_Float(keyframe.fovYDegrees) +
					" }";
				text += (keyIndex + 1u == shot.keyframes.size()) ? "\n" : ",\n";
			}
			text += "        ]\n      }";
		}
```

### 검증

- 프리 카메라로 원하는 프레이밍을 잡고 `Append Key From Camera`를 세 번 눌러
  키 세 개를 만든 뒤 저장하고 재로드했을 때 같은 값이 남는지 본다.
- `Play 2 (original)`을 눌러 카메라가 키를 따라 움직이는지, 끝나면 프리 카메라로
  돌아오는지 사용자가 화면으로 확인한다.

---

## G05. 트리거가 컷신 뒤에 플레이어를 옮기게 한다

### 왜 서버인가

플레이어 위치는 Server authority다. "컷신이 끝나면 옮긴다"를 Client가 판단해 보내면
그 경계를 깬다. 컷신 길이는 저작 데이터이므로 Server가 그 길이만큼 tick을 세면 된다.

### 파일 역할과 위치

- `Tools/WorldPipeline/Publish-WorldGameplay.ps1` — 저작 문서 검증과 bootstrap 생성.
- `Server/Public/WorldBootstrap.h` — 액션 구조체.
- `Server/Private/WorldBootstrap.cpp` — bootstrap 파서.
- `Server/Public/ServerTriggerSystem.h`, `Server/Private/ServerTriggerSystem.cpp` — 판정과 실행.

### 데이터 형식

`triggerBox.events`에 이벤트 여러 개를 허용하고, 각 이벤트에 선택 필드 `delayMs`를 더한다.
`Gameplay.world.json`의 `formatVersion`을 6에서 7로 올린다.

```json
{
 "placementId": "1Stage_Final",
 "kind": "triggerBox",
 "position": [27.7999992, 0.449999988, -67.6999969],
 "yawDegrees": 123.5,
 "enabled": true,
 "halfExtents": [6, 1, 1],
 "triggerOnce": true,
 "events": [
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_8T6_00" },
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_8T6_01" },
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_8T6_02" },
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_8T6_03" },
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_8T6_04" },
  { "type": "playSequence", "sequenceInstanceId": "world.sequence.instance.original_book" },
  { "type": "movePlayer", "targetPosition": [-2, 1.299, 730], "durationSeconds": 0, "arcHeight": 0, "delayMs": 6600 }
 ]
}
```

불변식.

- `delayMs`가 없으면 0이다. 즉 기존 문서는 그대로 유효하다.
- `delayMs`는 0 이상 60000 이하다.
- 지연 액션은 진입한 player 한 명에게만 걸린다. 지연 중 그 player가 방을 떠나면 취소한다.
- `triggerOnce` 판정은 지금처럼 첫 액션이 성공한 시점에 확정한다. 지연 액션이 남아 있어도
  트리거는 다시 발화하지 않는다.

### 선언

`Server/Public/WorldBootstrap.h`의 `WORLD_TRIGGER_ACTION`에서 `std::string strTargetId;`
아래에 추가한다.

```cpp
		/* Milliseconds to hold this action after the trigger fires. A cutscene
		   action and the move that follows it live in one trigger, so the room
		   keeps the wait instead of a Client reporting that a sequence ended. */
		std::uint32_t iDelayMs = 0u;
```

`Server/Public/ServerTriggerSystem.h`의 클래스 private 영역에 추가한다.

```cpp
		struct PENDING_ACTION final
		{
			SESSION_ID iSessionId = INVALID_SESSION_ID;
			LostArk::Shared::WORLD_ID eTargetWorldId =
				LostArk::Shared::WORLD_ID::END;
			WORLD_TRIGGER_ACTION Action;
			std::uint32_t iDueTick = 0u;
		};
		std::vector<PENDING_ACTION> m_PendingActions;
```

### 함수 한 줄 책임

- `Evaluate_Entries` — 진입을 판정하고 액션을 실행한다. 이번 G에서 지연 액션은 큐에 넣는다.
- `Flush_PendingActions` (신규) — 만기된 지연 액션을 같은 실행 경로로 흘려보낸다.
- `Cancel_PendingActions` (신규) — 세션이 떠나면 그 세션의 지연 액션을 버린다.

### 흐름

```text
Evaluate_Entries
→ 액션 반복
→ iDelayMs == 0 이면 기존처럼 즉시 실행
→ iDelayMs > 0 이면 m_PendingActions에 {세션, 액션, 만기 tick} 추가하고 fired = true
→ triggerOnce 확정은 기존 그대로

Flush_PendingActions(현재 tick)
→ 만기된 항목만 순서대로 꺼낸다
→ 해당 세션의 player가 아직 방에 있는지 확인
→ 없으면 버린다
→ 있으면 Evaluate_Entries와 같은 실행 분기를 호출한다
```

`CGameRoom`은 트리거 평가 직후 `Flush_PendingActions`를 호출하고,
플레이어 퇴장 처리에서 `Cancel_PendingActions`를 호출한다.

### publisher 교체 블록

`Tools/WorldPipeline/Publish-WorldGameplay.ps1:546`의

```powershell
			if ($events.Count -gt 1 -or ($placement.enabled -and $events.Count -ne 1)) {
```

을 다음으로 교체한다.

```powershell
			if ($events.Count -gt 16 -or ($placement.enabled -and $events.Count -lt 1)) {
```

그리고 각 이벤트 검증에서 `delayMs`를 선택 필드로 허용하고 0~60000 범위를 강제한 뒤
bootstrap 행에 액션마다 `delayMs`를 덧붙인다. bootstrap 행 형식이 바뀌므로
`Server/Private/WorldBootstrap.cpp`의 payload 개수 검사도 같은 변경 단위에서 올린다.

### 검증

- `Publish-WorldGameplay.ps1 -Mode Validate`가 기존 문서와 새 문서를 모두 통과한다.
- `Server.exe --contract-test`로 트리거 계약 테스트를 돌린다.
- 실제 Server+Client로 1Stage 끝 트리거를 밟아 컷신이 돌고 6.6초 뒤 아레나로 옮겨지는지
  사용자가 확인한다.

---

## G06. 데이터 저작

### 카메라 트랙

`shot.original`에 `cameraTrack`을 넣는다. 키프레임은 맵툴에서 프리 카메라로 잡아
`Append Key From Camera`로 만든다. 원작 카메라는 이 구간 내내 고정이므로
(`cameraactor_18`의 7.759초 키가 `constant`), 원작을 그대로 옮기면 움직임이 없다.
따라서 이번 연출은 원작 프레이밍을 시작점으로 삼고 사람이 새로 만든다.

시작 프레이밍의 근거값은 다음과 같다. 책 기준 수평 18.63m, 높이 1.95m, 화각 50도.
`BOOK_POSE (0, -1.41, 737.28)` 기준으로 eye `(15.849, 0.539, 727.685)`.

### 트리거

`1Stage_Final`을 G05의 형식으로 바꾼다. `circusfinale`은 지금 이 트리거가 유일하게
재생하는 시퀀스이므로, 컷신으로 교체할지 함께 둘지는 저작 판단이다. 이 PLAN은
컷신 여섯 개와 지연 `movePlayer` 하나로 바꾸는 안을 기본으로 한다.

`movePlayer`의 목표는 `playerSpawn stage.kakul.sl04`의 위치 `(-2, 1.299, 730)`이다.
지연값 6600ms는 컷신 실제 길이 6439ms에 여유 161ms를 더한 값이다.

### 검증

- `Publish-WorldGameplay.ps1 -Mode Publish` 성공.
- `Publish-MapAuthoring.ps1 -Mode Publish` 성공.
- 새 `composition.presentation` 도메인이 월드 시퀀스 쿼터니언 규격을 검사하므로
  카메라 문서 변경 후에도 Client pre-build 전체가 PASS해야 한다.

---

## G07. 종료 조건

```text
Client Debug 빌드 성공
Publish-MapAuthoring, Publish-WorldGameplay 두 publisher PASS
Server contract test PASS
맵툴 Play 2에서 카메라가 키를 따라 움직이고 끝나면 돌아온다 (사용자 확인)
1Stage 끝 트리거에서 컷신 재생 후 캐릭터가 아레나에 등장한다 (사용자 확인)
잘못된 카메라 문서 4종이 로드 거부되고 기존 샷이 보존된다
RESULT에 실행한 검증과 미검증을 분리 기록
```

## 남은 경계

- 원작 카메라는 이 구간에서 움직이지 않는다. 이번 연출은 원작 재현이 아니라 새 저작이다.
- 파티 4인에서 지연 `movePlayer`는 트리거를 밟은 한 명에게만 걸린다. 전원 이동이
  필요하면 별도 수직 슬라이스다.
- 컷신 도중 플레이어 입력을 막는 장치는 이 PLAN 범위가 아니다. 카메라만 넘어간다.
