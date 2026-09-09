# 2026-09-09 베른 입장 카메라 국소 조명 결과

대응 계획: [구현 계획](2026-09-09_BERN_ENTRANCE_LOCAL_LIGHT_IMPLEMENTATION_PLAN.md).

## G00. 실제 반영 범위

베른 입장 스폰부터 기존16초 카메라 경로 주변을 비추는 원본 SL00 Point10개를 `PROJECT_AUTHORED` direct 대체로 저작했다. 원본 설치 package와 component 직렬화 payload를 읽었고 Client/UI는 실행하거나 조작하지 않았다. 지형42개 가시성, 재질, 전체 map load scope, 방향광/fog, 카메라 JSON과 gameplay spawn은 수정하지 않았다.

| 원본 component | Client 좌표(m) | 반경(m) | 밝기 | falloff | 같은 GUID의 source baked component 수 | 프로젝트 대체 field |
|---|---|---:|---:|---:|---:|---|
| SL00:859 `pointlight_14_lc` | (133.542, 36.104, -163.748) | 12 | 0.2 | 2 | 113 | falloffExponent |
| SL00:861 `pointlight_18_lc` | (157.530, 34.764, -90.803) | 10.24 | 0.4 | 2 | 105 | rangeMeters, falloffExponent |
| SL00:862 `pointlight_19_lc` | (148.807, 34.764, -97.700) | 10.24 | 0.4 | 2 | 111 | rangeMeters, falloffExponent |
| SL00:863 `pointlight_1_lc` | (147.554, 44.715, -159.891) | 3 | 2 | 2 | 16 | falloffExponent, color |
| SL00:865 `pointlight_22_lc` | (125.282, 39.036, -154.189) | 12 | 0.3 | 2 | 112 | falloffExponent |
| SL00:866 `pointlight_27_lc` | (138.740, 36.104, -163.748) | 12 | 0.6 | 2 | 107 | falloffExponent |
| SL00:868 `pointlight_53_lc` | (127.368, 35.924, -161.650) | 12 | 0.3 | 2 | 125 | falloffExponent |
| SL00:869 `pointlight_55_lc` | (141.153, 55.916, -168.050) | 5 | 1 | 4 | 83 | brightness |
| SL00:870 `pointlight_64_lc` | (132.833, 55.916, -168.050) | 5 | 1 | 4 | 90 | brightness |
| SL00:872 `pointlight_9_lc` | (123.999, 30.458, -98.901) | 12 | 0.5 | 2 | 107 | falloffExponent |

전체 source는 `LV_BER_BERNCASTLE_T_*`36package340light다. 후보는 상시 PS/SL304개이며 EVENT/SCENE36개는 제외했다. eye/lookAt 선분과 spawn에 원본 sphere+12m 프로젝트 관찰 여백이 닿는10개만 사용한다. 스폰 바로 옆의 원본 local light는 없어 새 project key/fill은 만들지 않았다. 원본 Color alpha0은 광원 opacity가 아니므로 output alpha1로 해석했다. 명시되지 않은 radius1024cm/brightness1/falloff2/white는 프로젝트 값이다.

10개 source GUID는 native baked component와 join된다. 현재 베른 runtime mapmaterials/placementLighting0과 모델 UV1 부재를 함께 확인했으므로 이번에 중복될 제품 RNM 기여는 없다. 향후 이 구간 baked 연결 시 같은 GUID direct 대체를 제거하거나 수신기여를 분리해야 한다. 현재 direct는 원작의 baked 조명 경로가 그대로 실행된다는 뜻이 아니다.

## G01. 구현된 호출 경로

- `Data/Maps/Authoring/LV_BER_BERNCASTLE/LV_BER_BERNCASTLE.maplights.json`: v2,10enabled Point. stable ID에 source SL00/export를 넣었다.
- `Data/Maps/MapCatalog.json`: Bern sourceLights/lights canonical pair 추가.
- `Client/Public/Level_Bern.h`: light runtime 강한 owner와 단발 오류 보고 상태 추가.
- `Client/Private/Level_Bern.cpp`: map 초기화 뒤 JSON을 local shared runtime으로 stage한다. 실패하면 map을 clear하고 E_FAIL을 반환한다. Level 초기화 전체 성공 뒤 멤버를 commit하고 Update가 기존 frame provider에 제출한다. Level destructor가 문서를 clear하고 owner를 해제한다.

C++ 변경은 UTF-8 BOM없음/CRLF를 유지했다. 신규 C++ 파일은 없으며 JSON의 Client None/filter 등록은 통합 담당 root가 수행한다.

## G02. 실행한 검증

| 확인 | 결과 |
|---|---|
| 원본36package light/재질별 구조 read | package decode error0,light property parse error0 |
| `light_resources_pipeline.py --mode Validate --source <Bern maplights> --map-lights-area LV_BER_BERNCASTLE` | PASS,lightCount10 |
| `Publish-MapAuthoring.ps1 -AreaId LV_BER_BERNCASTLE -Mode Validate` | PASS,shard-set13,placement50,017,검증파일29 |
| MapCatalog/new light JSON parse | PASS |
| 해당 C++/H/Catalog/PLAN `git diff --check` | PASS |
| 실제 runtime publish | root Publish/Check PASS,13shards·50,017placements·29files,source/runtime light10 일치 |
| C++ compile/Product link | root Client Debug x64 ClCompile PASS. 전체 제품 shader compile/link 미실행 |
| 실제 Client frame light 제출/성능/visual | 미실행,사용자 확인 필요 |

하위 작업 시작 시 Client PID17652/Server PID38604는 종료하지 않았다. 이후 root가 기존 publisher로 runtime 문서를 생성하고 Check를 통과했다. 마지막 root 점검에서 Client/Server는 실행 중이지 않았다.

## G03. 남은 실행 확인

root가 None/filter 등록, Client 최소 compile, Bern Publish/Check를 끝낸 뒤 사용자는 새 Client의 Lobby -> Bern으로 진입한다. 기존16초 카메라 후반의 계단·광장 주변 표면과 ESC skip/재입장 시 빛 누적 여부를 직접 확인한다. source-ready와 runtime-ready,사용자 visual 판정을 분리한다. 중간 임시 조사 파일은 `C:/Users/user/AppData/Local/Temp/lostark-20260909-audit`에 보존했다.
