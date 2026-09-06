# 2026-08-28 발탄 클리어 컷신 카메라 계획

## 1. 목표

첨부 영상 `발탄 클리어 .mp4`의 실제 클리어 연출 구간을 기준으로 발탄 최종 사망 시 모든 클라이언트가 같은 서버 `DEAD` 시작 틱에서 카메라 컷신을 재생하고, 종료 뒤 일반 플레이 카메라로 복귀하게 한다.

이번 범위는 기존 등장 컷신과 동일하게 카메라 무빙과 컷 전환이다. 새 보스 애니메이션, 이펙트, 자막, 보상 UI는 추가하지 않는다.

## 2. 확인한 현재 호출 흐름

```text
Server Valtan HP 0
-> WORLD_ENTITY_ACTION::DEAD + actionStartTick 복제
-> CLevel_ValtanArena가 사망 입력 구성
-> CValtanCinematicCameraController가 deathCue 선택
-> Camera Tool과 제품 런타임이 같은 ValtanCinematicCamera.json 샘플러 사용
-> cue 종료 시 기존 플레이 카메라 복귀
```

서버는 카메라 asset ID를 소유하지 않는다. 이미 권위 있는 `DEAD` edge와 `iActionStartTick`을 복제하므로 새 서버 패턴이나 별도 로컬 타이머를 만들지 않는다.

## 3. 영상 실측

- 원본 길이: 약 20.37초, 30 FPS
- 일반 전투에서 클리어 컷 진입: 4.4667초
- 저각 근접에서 넓은 전경으로 하드 컷: 7.7333초
- 섬광 안에서 쓰러지는 중거리 구도로 전환: 약 11.7333초
- 일반 플레이 화면 복귀: 18.4333초
- 실제 카메라 cue 길이: `13,967ms`

상대 cue 시간은 다음 세 구간으로 고정한다.

```text
0..3266ms       낮은 근접 저각 접근
3267..7266ms    아레나 전체와 폭발을 담는 넓은 전경
7267..13967ms   쓰러지는 발탄과 빈 아레나를 남기는 중거리 이탈
```

두 컷 경계는 `3266/3267ms`, `7266/7267ms`의 인접 keyframe으로 표현한다. 전체 보간은 `LINEAR`로 두어 하드 컷 경계에서 spline overshoot가 생기지 않게 한다.

## 4. 데이터 계약

수정 정본은 `Data/Encounters/Valtan/ValtanCinematicCamera.json`의 기존 stable death cue `camera.valtan.clear.wide`다. ID를 바꾸지 않고 duration과 keyframe만 확장한다.

```json
{
  "cueId": "camera.valtan.clear.wide",
  "durationMs": 13967,
  "interpolation": "LINEAR",
  "easing": "LINEAR",
  "shakeAmplitude": 0,
  "shakeDurationMs": 0,
  "keyframes": [
    { "sceneId": "camera.valtan.clear.wide.scene.01", "timeMs": 0, "eye": [148.03, 26.4, -112.56], "lookAt": [156.03, 27.2, -122.06], "fovYDegrees": 38 },
    { "sceneId": "camera.valtan.clear.wide.scene.02", "timeMs": 1200, "eye": [149.03, 26.1, -112.36], "lookAt": [156.03, 27.0, -122.06], "fovYDegrees": 40 },
    { "sceneId": "camera.valtan.clear.wide.scene.03", "timeMs": 2400, "eye": [150.03, 25.9, -112.26], "lookAt": [156.03, 26.8, -122.06], "fovYDegrees": 42 },
    { "sceneId": "camera.valtan.clear.wide.scene.04", "timeMs": 3266, "eye": [151.03, 25.7, -112.16], "lookAt": [156.03, 26.6, -122.06], "fovYDegrees": 44 },
    { "sceneId": "camera.valtan.clear.wide.scene.05", "timeMs": 3267, "eye": [156.03, 48.0, -88.06], "lookAt": [156.03, 23.8, -122.06], "fovYDegrees": 64 },
    { "sceneId": "camera.valtan.clear.wide.scene.06", "timeMs": 4700, "eye": [154.53, 46.5, -90.06], "lookAt": [156.03, 23.8, -122.06], "fovYDegrees": 62 },
    { "sceneId": "camera.valtan.clear.wide.scene.07", "timeMs": 6100, "eye": [153.03, 44.5, -92.06], "lookAt": [156.03, 23.7, -122.06], "fovYDegrees": 59 },
    { "sceneId": "camera.valtan.clear.wide.scene.08", "timeMs": 7266, "eye": [151.53, 42.5, -94.06], "lookAt": [156.03, 23.6, -122.06], "fovYDegrees": 56 },
    { "sceneId": "camera.valtan.clear.wide.scene.09", "timeMs": 7267, "eye": [143.03, 28.5, -105.06], "lookAt": [156.03, 25.8, -122.06], "fovYDegrees": 47 },
    { "sceneId": "camera.valtan.clear.wide.scene.10", "timeMs": 8800, "eye": [141.53, 29.0, -106.56], "lookAt": [156.03, 25.2, -122.06], "fovYDegrees": 49 },
    { "sceneId": "camera.valtan.clear.wide.scene.11", "timeMs": 10300, "eye": [140.03, 29.6, -108.06], "lookAt": [156.03, 24.5, -122.06], "fovYDegrees": 52 },
    { "sceneId": "camera.valtan.clear.wide.scene.12", "timeMs": 11800, "eye": [138.53, 30.2, -109.56], "lookAt": [156.03, 23.8, -122.06], "fovYDegrees": 55 },
    { "sceneId": "camera.valtan.clear.wide.scene.13", "timeMs": 13967, "eye": [137.03, 31.0, -111.06], "lookAt": [156.03, 23.4, -122.06], "fovYDegrees": 58 }
  ]
}
```

## 5. 자동 검증

`Tools/ValtanPipeline/test_valtan_camera_tool_contract.py`와 `ActionPresentationTimelineHarness`에 다음 계약을 고정한다.

- death cue ID와 길이 `13,967ms`
- `LINEAR/LINEAR`, 13개 keyframe
- 첫/마지막 시간과 두 하드 컷 인접 시간
- 인접 컷 전후 위치가 실제로 큰 폭으로 바뀌는지
- Camera Tool과 제품 런타임이 같은 문서를 parse/serialize/sample하는지

검증 순서는 camera publisher validation, Python camera contract, Debug/Release ActionPresentationTimelineHarness, Debug/Release Client 빌드, JSON parse, `git diff --check`다.

