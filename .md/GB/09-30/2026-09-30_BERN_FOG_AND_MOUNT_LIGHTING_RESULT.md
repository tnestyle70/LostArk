# 베른 안개 토글·용 탑승 조명 결과

## G01. 소스 반영

Rendering Workbench의 기존 `Height Fog Enabled`를 유지했다. 지역 안개 선택 후 profile의 fog.enabled도 함께 확인하도록 RenderingProfileService를 수정했다. 기존에는 베른의 환경 region이 enabled=true인 Fog를 다시 적용하여 OFF가 무효였다. 이제 OFF는 지역 안개까지 끄고 ON은 기존 density·height·색·지역 설정과 blend로 돌아간다. Light Detail의 tooltip과 설명은 master 범위와 기존 Save Light/Publish Light 경로를 알려 준다. 새 Volumetric 기능·버튼·schema는 추가하지 않았다.

고대의 바다 9523의 skill 98521·98523에는 원본 directionalLightCues brightnessMultiplier=0.0001이 있다. MainApp이 이를 scene diffuse/specular 전체에 곱하던 project adapter를 제거했다. 일반 character presentation directional control은 그대로 scene 입력으로 전달한다. 원본 vehicle catalog/cue, 재질 TransColor, shake/effect/sound, flight·Server 상태는 변경하지 않았다. 이 코드 경로는 해당 skill cue 구간의 어두워짐 원인을 확인한 것으로, 모든 탑승 순간 화면을 직접 재현했다고 기록하지 않는다.

수정 파일은 MainApp.cpp, MainApp_RenderingLighting.cpp, RenderingProfileService.cpp 세 개다. 기존 작업의 dirty diff와 UTF-8 BOM 없음·CRLF를 유지했다. 새 C++ 파일이 없어 project/filter 추가는 없다. RenderingProfiles와 VehicleCatalog를 포함한 제품 Data/Resources/게시본은 이 작업에서 쓰지 않았다.

## G02. 실행한 검증

- 현재 Authored·runtime 각각 28개 profile을 읽었다. 지역 사용 profile 12개·총 56개 region 모두 base fog.enabled=true여서 이번 gate가 기존 저장 설정을 바꾸는 조합은 0개다. 베른 활성 profile은 기존 enabled=true·density=0.300000012·5개 region을 유지한다.
- 전체 profile JSON의 bool OFF→ON 의미 보존을 대조했다. 실제 C++ 호출 흐름에서 Update_Profile → Apply_ActiveProfile → Commit_Resolved가 presentation override와 지역 ID를 초기화하고, 다음 camera resolve가 같은 지역 수치를 선택함을 확인했다. 이는 소스·데이터 검토이며 C++/GPU 실행 검증이 아니다.
- MainApp의 scene 환경 인자에서 vehicle brightness 소비가 제거되고 일반 presentation control 및 ship/cinematic/region 조건이 보존됐는지 변경 전 bytes와 대조했다.
- `Publish-RenderingProfiles.ps1 -Mode Validate` PASS. 변경 범위 `git diff --check` PASS. C++ 세 파일의 UTF-8 BOM 없음·CRLF 보존 PASS.
- 수정 전 파일과 정확한 부분 변경 영수증은 `out/BernFogMountLighting20260930/baseline`, `source-edit-receipt.json`, `source-review.json`에 있다.
- 독립 검토자가 baseline 대비 세 파일에서 region 수치 보존·master gate·vehicle 전역 배율 제거·일반 presentation control 보존을 확인했고 소스상 문제를 발견하지 않았다. 컴파일·화면 검증과 구분한다.

## G03. 빌드와 화면 확인 상태

초기 빌드 보류 이후 사용자는 직접 빌드하여 검토 중이라고 알렸다. 에이전트는 C++/shader 컴파일·제품 링크·Client/UI 실행·프로세스 종료를 수행하지 않았고, 사용자 빌드의 성공 여부와 화면 결과는 아직 받지 않았다. 따라서 이 결과는 소스 반영과 정적 검증 완료이며 현재 실행 파일 적용을 확인했다는 뜻이 아니다. 베른 지역 안에서 Height Fog Enabled OFF/ON 및 Save/Publish/재진입, 용 Q/E 연출 구간의 맵 방향광 유지, 일반 캐릭터 presentation light와 영화·ship fog 동작의 사용자 화면 확인이 남아 있다.
