# Guardian ALT V 카메라 좌표계 결과

## 구현 상태

`guardian_camera_projection.py`의 카메라 eye/forward/up 변환을 동일 occurrence의
`decode_source_model_actor.py::source_actor`와 같은 UE `[x,y,z]` → runtime `[y,z,x]`로 맞췄다.
기존 카메라는 `[x,z,-y]`여서 뒤쪽 `-Z`에 배치된 용을 카메라만 `+X` 옆에서 바라봤다.
모델 occurrence, 일반 이펙트, Character yaw에는 회전 보정을 추가하지 않았다.

원본 Camera Move102에는 1.5~2초의 줌아웃과 FOV 38→90 변화가 이미 존재한다. 이 원본 곡선을
유지하고 타임라인에 `Close-up`(0~1500ms, 181 keys), `Zoom out`(1500~3800ms, 278 keys)을
분리했다. 경계 pose/FOV는 동일하며 원본 action STOP 3800ms를 유지한다. 경계의 같은 pose를
각 row에 포함하므로 총 459 keys이며 고유 시간 샘플은 기존 458개다.

설치 파일은 `Data/Effects/Sequences/effect.guardianknight.skill.49420.clip.0.full.restore.effectsequence.json`이다.
교체 직전 최신 bytes 재확인과 백업 후 camera 배열만 원자 교체했다. 백업은
`out/GuardianAltVCamera20260922/camera.before.json`이다. camera 이외 모든 field는 백업과 동일하다.
sceneBackdrop 요소 수정은 상위 통합 작업의 별도 소유 범위다.

## 검증 결과

`python -m unittest discover -s Tools/EffectPipeline -p test_guardian_owner_control_projection.py`
실행 결과 9개 PASS다. 기존 원본 visibility control 검사와 함께 축 대응, 원본 STOP,
줌아웃 거리/FOV, row 경계 연속성을 확인했다.

최종 JSON parse, ID 길이와 key 중복, row 비중첩, 첫/끝 key 시간, 459개 pose의 finite 좌표,
forward/up 단위 길이와 직교를 확인했다. 이는 `CEffectRecoveryCamera::Validate`의 주요 데이터
조건을 수치로 대조한 것이며 C++ 실행 검사를 대신하지 않는다.

| 시각 | eye | lookAt | FOV |
|---|---|---|---|
| 0ms | `[0.053726, 1.042611, 1.842632]` | `[0.047974, 1.033791, 0.842687]` | 30.0 |
| 1500ms | `[0.056173, 0.878026, 2.120372]` | `[0.047353, 0.877929, 1.120410]` | 37.9998 |
| 3800ms | `[0.003263, -1.012422, 7.730097]` | `[-0.035567, -0.487506, 6.879829]` | 91.0023 |

원본 용 anchor `[-0.4,-2,-11]` 방향과 카메라 forward의 dot은 시작 0.974666,
종료 0.822094다. source actor 뒤쪽을 향함을 확인한 수치이며 실제 용 mesh의 가시성과
구도 성공으로 기록하지 않는다. 원본 음수 카메라 높이는 임의 clamp하지 않았다.
대상 파일의 `git diff --check`는 통과했다.

## 남은 경계

Client/UI 실행 및 화면 캡처는 하지 않았다. 전체 빌드와 C++ 카메라 로드는 상위 통합 검증에서
확인하며 원작 화면과 최종 구도는 사용자가 확인한다. 원본 3800ms 이후에 별도 줌아웃 또는
후속 카메라를 추측해서 추가하지 않았다.

사용자가 지적한 Play All과 gameplay의 용 90도 방향 차이는 이번 조사에서 독립 원인을
확정하지 못했다. preview `PresentationRoot`와 gameplay `root`의 실제 CCharacter 경로는
동일 presentation yaw를 쓰며, 기본 상태에서는 scale 차이만 확인됐다. Guardian ALT V
root notify TRS는 identity이고 모델 pose는 `Local * RootWorld`를 공통 소비한다.
Play All에서 정상인 용을 일괄 회전하는 변경은 적용하지 않았다. 후속 실행에서 두 경로의
실제 owner matrix, 캐릭터 facing, actionFacing 및 occurrence transform을 함께 비교해야 한다.
