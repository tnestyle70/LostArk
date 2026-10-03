# Rendering Workbench 기법별 A/B 연결 결과

## G00. 원인과 실제 구현 범위

SSAO·SSGI·SSR·PCF·RNM·IBL·Bloom·FXAA는 기존 renderer와 shader에 연결돼 있었다. 기법 사전은
설명만 표시했고, 원본 PBR 간접광과 source tone·grading 묶음은 독립 세션 필드가 없었다.
옛 `Selected Reference A/B Start`는 실제 A/B 대신 품질을 초기화하고 exposure/FXAA를 바꿨다.

Character Select에서 근접 굴곡 음영은 SSAO, 표면 요철은 normal 강도, baked 명암은 RNM이다.
원본 SH·cube·BRDF 복원은 지원 PBR 재질의 차가운 분위기와 관련된 기존 경로이며 전체 scene 색을
지정하는 sky-blue preset은 아니다. 원본 후처리는 tone과 grading을 함께 선택한다.
SSGI·SSR은 marker3 MapPBR만 수신한다. marker5 캐릭터·다른 재질 ABI를 무리하게 확대하지 않았다.

## G01. 반영 코드

- RenderingProfileService의 whitelist를 41→43 필드로 확장했다. 기존 ID·순서를 보존하고
  `environment.sourcePbrIndirect.enabled`, `quality.sourcePostProcess.enabled`를 끝에 추가했다.
- 원본 간접광은 현재 environment의 bool만 소유한다. cube·SH·색·리소스를 보존하고 fallible
  quality/shadow/fog/material 적용 성공 후 commit한다. source postprocess도 enabled만 복원한다.
- LUT ON은 동시에 적용할 source postprocess 후보의 활성 여부와 기존 LUT 입력을 확인한다.
- Benchmark의 recipe는 31→34개다. 기법 사전의 지원 항목을 실제 recipe에 연결하고 선택만으로는
  설정을 바꾸지 않는다. 버튼으로 현재 A 보관/B 준비 또는 기존 B 대체를 명시한다.
- 새 selector는 실제 측정 조건에 포함하되 해당 bit를 비교할 때만 공통조건에서 제외한다.
  cube·SH·LUT·curve·색·gamma와 다른 입력은 계속 조건 차이로 검출한다.
- SSAO/GTAO, Height Fog/Volumetric, SSR/Planar 지원 상태를 분리했다. 원본 간접광 ON에서
  억제되는 cube diffuse와 source postprocess OFF에서의 LUT 조절은 이유를 표시한다.
- 오래된 SH/hemisphere 미복원 안내를 교정하고 잘못된 A/B 버튼을 현재 상태 세션 시작으로 교체했다.

기존 C++ 파일만 수정했으므로 vcxproj/filter 신규 등록은 없다. Engine shader 변경, 프로필 저장,
publisher 실행, 사용자 rendering tuning 변경은 없다.

## G02. 검증 증거

- RenderingProfileService.cpp, MainApp.cpp, RenderingBenchmark.cpp 및 공유 ReferenceGuide 소비자인
  ProfilerTool.cpp의 분리 Debug 컴파일 통과. UTF-8 TU는 프로젝트의 `/utf-8` 설정을 사용했다.
- 실제 Service 함수와 public 타입 기반 native 검증 **93 checks / 0 failures**.
  read/validate/stage/ON·OFF/A 복귀, LUT 조합, 4개 renderer setter의 적용·복원 실패와 재시도,
  profile/level/region/video owner 변경, 다른 환경/후처리 필드 보존을 검증했다.
  증거: `out/RenderingAB20261004/service-verification.json`.
- 실제 ComparisonConditions/Current_Conditions/ChangedConditionFields 기반 native 검증
  **70 checks / 0 failures**. 선택 selector만 공통조건에서 빠지고 실제 조건 및 다른 입력 변경은
  검출됨을 확인했다. 증거: `out/RenderingAB20261004/fingerprint-review/run.log`.
- 실제 Benchmark 세션 시작/recipe ID 준비/B 교체/적용/기준 채택/종료와 Service transaction 기반
  **66 checks / 0 failures**. capture/live/restoration 소유권 거절, 잘못된 ID, 실패 시 기존 B 보존,
  PP/LUT 의존성, 원본 간접광 ON에서 cube 비교 거절과 OFF 기준 채택 후 허용을 확인했다.
  기법 사전 14개·Reference 5개 연결은 34개 recipe ID에 모두 존재한다.
  증거: `out/RenderingWorkbench20261004/ui/session_receipt.json`.
- 위 검증은 Game/User 경계 mock을 사용했다. 실제 Client 화면이나 GPU 화질 확인으로 대신하지 않는다.

## G03. 제품 반영과 남은 확인

사용자가 계속 편집 중이라고 확인하여 제품 EXE 링크·배포는 보류했다. 분리 컴파일과 자동 검증,
소스 PR 병합을 제품 EXE 적용 완료로 설명하지 않는다. Client/Server 종료 후 Debug Product를
진행한다. 사용자 저장 rendering authored/runtime과 camera/composition/mapplacements 및 실시간
Effect 저장의 무관한 변경은 그대로 보존한다. git diff --check 통과.

Client/UI를 자동 실행하거나 조작하지 않았다. 최종 Character Select에서 동일 카메라로 SSAO,
원본 PBR 간접광, Source tone + grading을 각각 A/B하는 화면 판정은 사용자 확인 범위다.
Lumen/DXR/DDGI/GTAO/Planar/Volumetric 등 미구현 기법을 새로 구현했다고 표시하지 않는다.

## 후속 통합 제품 반영

2026-10-04 사용자 종료·빌드 승인 뒤 Debug와 Release Product 빌드·배포를 모두 완료했다.
이 문서의 변경도 해당 실행 파일에 포함된다. 두 receipt와 검증 경계는
[Movie 통합 반영 결과 G04](2026-10-04_MOVIE_CAMERA_SAVED_POSE_REBASE_RESULT.md#g04-통합-제품-빌드)에 기록했다.
Client/UI를 자동 실행하지 않았으며 최종 화면 확인은 사용자에게 남는다.