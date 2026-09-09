# 맵 원본 재질 확대와 비교·레퍼런스 모델 연결 구현 계획

작성일: 2026-09-09. 상태: 실행 체크포인트로 범위 고정. G02 A/B 비교와 G03 인형 모델·30개 clip을 반영했다. 맵 확대·캐릭터 환경반사·발탄 Composition 확장은 조사/계획 단계다. 기존 공유 변경과 셰이더 분리·Play All 최적화를 보존한다. 실제 검증은 대응 RESULT를 따른다.

## G00. 입력과 담당 경계

쿠크는 전체 배치에서 실제 사용한 재질을 조사하고, 발탄은 전투 아레나와 카메라 주변, 베른은 현재 spawn과 BernEntranceCamera의 경로 주변을 대상으로 한다. 기존 쿠크 바닥의 원본 shader·uniform·texture 대응 방법을 재사용한다. 서로 다른 원본 material family에 바닥 수식을 공통 적용하지 않는다. 배치 ID·Transform·사용자가 편집한 광원과 노출은 유지한다.

발탄 Composition은 기존 `2026-09-09_VALTAN_COMPOSITION_AUTHORING_PARITY_IMPLEMENTATION_PLAN.md`, 캐릭터는 `2026-09-09_CHARACTER_SCALE_AND_MATERIAL_PARITY_IMPLEMENTATION_PLAN.md`를 갱신하며 구현한다. 차원술사 1.5배는 이미 적용된 상태다. 이 문서는 맵·A/B·인형 모델 경계만 소유한다.

## G01. 맵 표면의 실제 입력 연결

현재 `Data/Maps/Imported`의 catalog와 `Data/Maps/Authoring`의 placement를 원본 component·material slot에 join한다. 재질별 parent·static switch·uniform·texture와 추가 UV·정점색 사용을 확인한다. 원본을 증명한 branch만 기존 `CModel -> CMaterial -> CMapAssetRenderUtils -> Shader_MapMaterialSurface`에 연결한다. 누락된 필수 입력을 흰색 texture나 가짜 반사로 채우지 않는다. 설치한 Resources와 실제 연결하지 못한 branch를 결과에서 구분한다.

기존 program7의 발탄 석재·baked lighting, program1/2의 쿠크 바닥, 다른 원본 계열의 계약을 유지한다. 새 family가 필요하면 같은 runtime과 기존 mapmaterials schema에 명시적으로 추가하고 reader·shader·publisher·진단을 함께 변경한다. runtime 배포는 기존 Map publisher를 사용한다.

## G02. Rendering Benchmark의 A/B 기록

`Client/Private/MainApp.cpp`의 Floor Materials를 전체 Map Materials 비교로 확장한다. 기존 source-material 스위치를 A/B 상태의 단일 정본으로 사용한다. `RenderingBenchmark.h/.cpp`는 각 측정에 material mode, debug view, viewport·view/projection과 scene/quality를 기록하고 동일 조건의 A/B 결과를 비교한다. 측정 중 camera/설정이 바뀌면 비교 유효성을 명시한다. 실제 사용자 입력으로 Capture를 시작하며 에이전트가 Client를 실행하지 않는다.

기존 CPU/GPU history 지연·activation frame 제외 규칙은 유지한다. 비교는 같은 scene의 source-material 여부만 달라야 한다. GPU 표본이 없을 때 0 ms 개선으로 표시하지 않는다. JSON 결과는 기존 BenchmarkCaptures에 저장한다. 새 C++ 파일은 필요하지 않으며 프로젝트/filters 재배치는 하지 않는다.

## G03. 네 다리 화염 인형 Resources와 preview

원본 `mn_cdmd_00.Mesh.MN_CDMD_00_SK_LOC_INT`, `MN_CDMD_00.Ani.MN_CDMD_00_Ani`와 `Att_Battle_2_01`을 기존 추출·cook 경로로 확보한다. 앞선 UModel 오류는 원본 읽기와 도구 차이를 먼저 진단한다. 설치 게임 파일은 변경하지 않는다. skeleton·clip·material texture를 실제 decode하고 기존 WModel/CModel 계약으로 설치한다.

등록은 `LV_LUT_MIDNIGHTC_ED.worldsequences.json`의 기존 objectResources·templates·instances에 stable ID로 추가한다. Object Resources에서 선택하고 native clip을 편집·재생한 뒤 Composition Resources의 World에서 Append·Preview·Save할 수 있도록 기존 소비자를 연결한다. 모델 animation과 별도 particle 화염은 구분하며 화염을 연결하지 않았으면 결과에 명시한다. 오류는 해당 항목에 표시하고 기존 draft를 보존한다.

## G04. 검증과 완료 경계

원본 입력·추가 채널·geometry 보존 수치 검사, 변경 JSON/XML parse, 해당 publisher 검사·배포, 변경 C++ 최소 컴파일과 shader 수치 검사 후 Product Debug를 한 번 통합 빌드한다. 비교 기록·resource 선택·Save 실패 보존처럼 실제 달라지는 계약만 검증한다. 사용자 화면·FPS·원작 일치 판정은 자동 PASS로 기록하지 않는다. Resources 상대 경로와 물리 설치 위치, Drive 미전달 상태를 RESULT에 남긴다.
