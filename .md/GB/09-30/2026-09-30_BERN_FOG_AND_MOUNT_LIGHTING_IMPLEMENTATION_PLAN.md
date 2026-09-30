# 베른 안개 토글·용 탑승 조명 구현 계획

## G00. 현재 저장본과 변경 범위

베른의 `scene.bern.neutral-day.v1`은 fog.enabled=true, density=0.300000012와 다섯 environment region을 사용한다. 기존 Height Fog 체크박스는 base fog.enabled를 바꾸지만 camera region이 enabled=true인 지역 Fog를 다시 적용하여 지역 안에서는 OFF를 유지하지 못한다. Authored·runtime 각 28개 profile 중 region이 있는 12개는 모두 fog.enabled=true다. 따라서 기존 저장값을 바꾸지 않고 이 필드를 profile 전체의 fog master로 소비해도 현재 화면 설정은 유지된다.

고대의 바다 9523의 98521·98523 원본 directionalLightCues에는 brightnessMultiplier=0.0001이 있다. 현재 MainApp은 그 값을 전체 scene directional diffuse/specular에 곱한다. 이는 09-22 RESULT가 명시한 project adapter이며 원본 retail scheduler와 같다고 확인한 경로가 아니다. 이번 변경은 이 탈것 입력을 전역 scene 조명으로 적용하는 연결을 제거하여 맵 조명이 용 동작 때문에 어두워지지 않게 한다. 일반 character presentation directional control·Light Workbench 비교·지역 조명은 유지한다.

첨부 두 화면은 기존 height fog와 용 탑승 시 외형을 확인하는 참고 자료다. 실행 중 상태를 직접 조작하지 않고, 사용자 지시에 따라 빌드·Client/UI 실행은 보류한다.

## G01. Rendering Workbench와 저장된 안개 입력

사용자가 기존 Height Fog Enabled 버튼을 확인했으므로 새 Volumetric Fog 버튼은 추가하지 않는다. `Client/Private/MainApp_RenderingLighting.cpp::RenderSceneProfileDetail`의 기존 체크박스가 profile 전체 안개의 master임을 설명한다. 기존 draft의 bool만 바꾸고 Update_Profile 검증·commit·rollback을 사용한다. 밀도·높이·색·지역·quality를 새 기본값으로 재설정하지 않는다. 기존 Save Authored/Publish Runtime의 field merge·freshness·writer lock·원자 교체가 저장을 소유한다. 새 schema, rendering option, H 멤버도 추가하지 않는다.

## G02. 지역 안개의 master 적용

`Client/Private/RenderingProfileService.cpp::Apply_CameraRegionEnvironment`는 selected region/base의 수치와 기존 blending을 그대로 사용한다. 최종 fog.bEnabled에 profile.Fog.bEnabled를 함께 적용한다. Update_Profile의 기존 Commit_Resolved가 지역 ID를 비우므로 OFF/ON 이후 현재 지역을 다시 resolve하고, presentation restore와 이후 cinematic/ship suppression도 기존 순서를 따른다. 지역 수치와 소스 JSON은 바꾸지 않는다.

## G03. 용의 일시 배율과 scene 조명 분리

MainApp의 environment 입력 구성에서 Get_VehicleDirectionalBrightness 결과를 scene multiplier로 합성하지 않는다. 일반 Get_PresentationDirectionalControl 결과만 기존 Apply_CameraEnvironment 인자로 전달한다. 원본 vehicle cue, shake, material TransColor, effect, sound, flight·Server 상태를 삭제하거나 바꾸지 않는다. 남아 있는 getter는 원본 cue 관찰 상태를 유지하며 전역 조명 소비자가 아니게 된다.

## G04. 검증과 종료 경계

수정 전 세 C++ 파일의 bytes·인코딩·dirty diff를 보존하고 정확한 변경 블록만 적용한다. 기존 UTF-8 BOM 없음·CRLF를 유지한다. 신규 C++ 파일이 없어 vcxproj/filter 등록은 필요하지 않다.

전체 profile의 enabled/region 조합, OFF→ON 수치 보존, 지역 진입·이탈 및 presentation 복원 순서, 원본 용 cue 배율과 scene 입력 분리, 변경 밖 bytes·데이터 보존을 소스·데이터로 확인한다. 관련 Rendering publisher Validate와 작업 범위 git diff --check를 실행한다. 사용자 지시대로 C++/shader 컴파일·링크·Client·UI는 실행하지 않으며, 소스 반영 완료와 제품 빌드·화면 미확인을 RESULT에서 구분한다.
