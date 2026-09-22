# 쿠크 2관문 클리어 포탈의 검은 배경막 수정

## G00. 목표와 실측

같은 재생 시각에 원래 카메라에서는 배우가 검고 포탈 안쪽에서는 밝게 보인다.
현재 배우 재질의 추가 point light 계산은 GPU 수치 검사에서 정상이다.
원인은 native3330의 검정 alpha 원통이 조명 이후 배우 앞에 합성되는 것이다.
실제 설치 모델의 9.5/12초 pose와 카메라에서 쿠크 전체, 거대 세이튼 정점 약97%의
앞에 원통 외벽이 있다. 이는 화면 픽셀 비율이나 최종 사용자 화면 PASS가 아니다.

원본 material의 unlit/translucent/one-sided 및 source placement는 현재 데이터와
일치한다. 원본 엔진의 화면 전체를 재현해 비교한 것은 아니므로 이 수정은
PROJECT_TUNED 배경막 처리다. 누락된 조명이나 원본 shader 식 복구로 기록하지 않는다.

## G01. 변경 범위와 데이터 흐름

- `Tools/KoukuSaydonPipeline/prepare_gate2_clear_portal_backdrop.py`: 설치된 기존
  원통의 triangle index 순서를 뒤집고 local X/Z만 2배로 넓힌 파생 WModel 후보를 생성한다.
  반지름은 8.75→17.5m이며 길이·위치, UV, normal, tangent, material은 보존한다.
  winding만 바꾸면 거대 세이튼 일부가 먼 벽을 통과해 남는 가림이 있어 반경도 조정한다.
- 파생 Resources ID:
  `Effect/KoukuSaydon/FullRestore/Meshes/fm_b_cylinder_002_gate2_clear_inward.wmodel`.
  공유 원본 WModel은 교체하지 않는다.
- `effect.kouku.sequence.lv_lut_midnightc_ed_scene02a.efseqact_matinee_10.1.effect.json`의
  `kouku.action.233d0d5adbba2933178bf071` 한 요소의 `meshModel` asset ID만 교체한다.
  기존 CModel 및 native3330/alpha depth-read 경로를 계속 사용한다.

현재 바깥으로 향한 앞벽 대신 원통의 안쪽에서 보이는 먼 벽만 그린다. 배우 앞의
검은 합성을 제거하면서 배우 뒤의 어두운 배경을 유지한다. 다른 포탈 요소와 조명,
재질, camera, animation 및 occurrence 시간은 바꾸지 않는다.

## G02. 검증과 적용 경계

원본/후보 WModel의 차이가 index winding과 vertex X/Z뿐인지 바이트 단위로 확인한다.
실제 배우 pose의 광선 교차를 후보에 다시 적용하고, 기존 배포 native3330 CSO를
사용하는 offscreen 수치 검사에서 검은 alpha 합성과 culling 차이를 확인한다.
후보 JSON parse와 stable element 외 필드 보존, 실제 shader/texture 소비 경로를 확인한다.
데이터와 Python 생성 도구만 바꾸므로 Product EXE/HLSL 빌드는 필요하지 않다.

사용자 편집 중 최종 반영은 AGENTS.md의 `편집 중 데이터 반영`에 따라 한 번 승인받는다.
그 뒤 최신 저장본을 다시 읽고 대상 필드만 병합한다. 교체 직전 hash 재검사, 백업,
원자적 교체 및 실패 시 자기 변경 rollback을 지킨다. Client 종료나 자동 Reload는 하지 않는다.
Resources 설치, 저작 파일 반영, 실행 중 메모리 재로드 및 사용자 화면 확인을 분리한다.
새 Resources는 Git 제외이며 팀 Drive 공유는 별도다.
