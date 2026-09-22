# Guardian ALT V 카메라 좌표계 구현 계획

## G00. 원본과 소비 경로 실측

`guardian_owner_controls.json`의 원본 Camera Move102와 FOV81은 1.5~2초에 거리 약 2m→7.5m,
FOV 38→90으로 줌아웃한다. 기존 458개 key 안에도 이 구간이 존재한다. 그러나 카메라 투영만
UE `[x,y,z]`를 `[x,z,-y]`로 바꿔 원작 전방을 runtime +X로 만든다. 동일 occurrence의
skeletal actor는 `[y,z,x]`로 바꿔 원작 전방을 runtime +Z로 만든다. 용이 -Z 뒤에 있는 동안
카메라는 +X 옆에서 바라보는 불일치다.

## G01. 변경

`Tools/EffectPipeline/guardian_camera_projection.py`에서 eye/forward/up 모두 skeletal actor와
동일한 `[y,z,x]` 축을 사용한다. 원본 parent transform·Hermite tangent·FOV와 3.8초 STOP은 유지한다.
타임라인에서 close-up과 zoom-out을 직접 확인할 수 있도록 원본 zoom-out 시작 1.5초에서 두 연속
camera row로 나누고 경계 pose는 같은 원본 샘플을 사용한다.

기존 owner control test에 전방/오른쪽/위 방향, 줌아웃 거리·FOV·경계 연속성과 원본 stop 구간을
검증한다. 최신 sequence JSON의 camera 배열만 백업·원본 재검사·원자 교체한다.
modelCues나 공용 Character yaw에 일괄 90도 보정을 넣지 않는다. Gameplay/preview 용 방향 차이는
실제 owner basis와 후속 사용자 화면에서 따로 판정한다.

## G02. 검증

기존 Python owner control test와 JSON 구조를 확인한다. source actor 뒤쪽 위치에 대한 camera forward
dot와 시야각을 수치로 대조한다. Native camera의 음수 높이를 임의로 올리지 않으며 sceneBackdrop
배경 대체는 상위 작업의 기존 renderer 계약을 사용한다. Client/UI는 실행하지 않는다.
