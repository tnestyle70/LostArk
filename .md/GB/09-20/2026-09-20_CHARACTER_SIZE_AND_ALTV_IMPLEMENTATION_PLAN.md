# 캐릭터 크기와 차원술사 Alt V 튜닝

## G00. 현재 기준

09-19 계획의 ScreenPost 방향별 수축 속도·회전 UI와 Codec·shader 연결은 현재 소스에 존재한다. 설치 저작 문서에는 새 튜닝 값이 저장되지 않아 회전 0 기본값으로 재생된다. 사용자 요청은 현재 화면을 기준으로 도화가 1.6배, 차원술사 0.7배, 광기 광대 0.7배로 해석한다. 현재 catalog admission scale 1.5/1.05를 다시 쓰지 않고 새 map 크기 배율의 초기값으로 이 요청을 적용한다.

## G01. 크기 저장과 소비

기존 ArenaCameraProfile JSON에 optional classSizeMultipliers, clownSizeMultiplier, marioSizeMultiplier를 추가한다. stable class 이름으로 저장하며 기존 문서도 새 초기값으로 읽힌다. 여섯 class와 두 광대를 Player Follow Camera의 Character Size 패널에서 조절하고 기존 Save/Reload를 사용한다. map의 크기 snapshot을 CCharacter가 공유하므로 로컬과 원격 캐릭터가 같은 값을 소비한다. Character root만 바뀌며 Server transform, collider, damage 범위는 바꾸지 않는다. 일반 광대는 무기를 숨기고 Mario만 기존 무기와 Mario admission height를 유지한다.

## G02. Alt V

기존 capture panel에 45도 정사각형·model center 프리셋을 제공한다. 기존 수축 방향·속도·회전과 동일 Apply/Save를 사용한다. 화면 Color와 Bloom은 같은 UV로 회전한다. 큐브의 기존 ALT178 captured-scene native material과 TRANSLUCENT 합성을 유지한다. full/tuning 문서는 최신 disk 기준 stable element ID·field 변경 후보를 만들며 사용자 저장본 확인 전에는 교체하지 않는다.

## G03. 검증

기존 파일 인코딩과 다른 세션 변경을 유지하고 새 C++ 파일은 만들지 않는다. 기존 프로젝트 등록을 사용한다. Product 증분 컴파일, profile의 직렬화·로드·잘못된 값 거부, candidate JSON parse와 field diff, git diff --check를 실행한다. Client/UI 실행과 화면 판정은 사용자에게 남긴다.

## G04. 사용자 화면 회귀: 캡처가 바닥으로 내려가는 현상

기존 G02의 model center 프리셋은 원본 복구값으로 확정된 값이 아니다. 설치 cube의 첫 pose는 캐릭터 root 원점에 있고 notify 036의 원본 transform은 아직 미해독이다. `captureUseModelCenter=true`를 켜면 그 발밑 중심을 캡처 끝점으로 선택한다. 또한 현재 ScreenPost의 일반 Transform은 target bounds 계산으로 덮여 위치·크기 튜닝이 실제 끝점에 전달되지 않는다.

정확한 ALT V capture/cube 연결에만 공통 카메라 기준 rig를 적용한다. model center를 끄면 같은 카메라 깊이의 화면 중심을 기준으로 하고, 켜면 기존 월드 모델 기준을 유지한다. ScreenPost의 기존 Transform position/rotation/scale을 캡처 끝점·다섯 원본 액자·실제 cube 및 그 bone attachment가 같이 소비한다. 카메라 축은 X 오른쪽, Y 위, Z 깊이다. Model Cue Local TRS는 추가 모델 미세조절로 남긴다. shrink duration과 네 방향 edge speed, UV offset, 45도 회전, frozen Color/Bloom pair와 native178 shader는 유지한다. 기존 Transform lerp/velocity는 capture 끝 시각에서 평가하여 cube 구간에 유지한다. 별도 asset·저장 schema·런타임 carrier를 만들지 않는다.

full/tuning 문서는 최신 파일과 hash를 보존한 뒤 해당 stable ScreenPost의 `captureUseModelCenter=false` 한 필드만 out 후보로 준비한다. 현재 실행 중인 사용자 편집 상태를 덮어쓰지 않으며 설치·publish는 root가 취합한다. 실제 설치 cube CModel과 production rig/projection 본문으로 바닥 기준 재현, 화면 중심 교정, position/scale/rotation/UV 이동, 비정상 camera 거절을 수치로 확인한다. 이는 프로젝트 튜닝 경로의 복구이며 미해독 원본 camera CB나 게임 화면 일치 완료로 기록하지 않는다.
