# 환경설정 텍스처 mip 품질 연결 구현 계획서

## G00. 목표와 현재 실측

사용자는 기존 환경설정에서 품질을 선택하면 실제 화면 품질이 바뀌도록 요청했다. 이번 기능은 `텍스처 품질`과 이를 포함하는 `일괄 설정`을 기존 GPU 표면 sampler까지 연결한다. 최상/상/중/하는 mip 최소 레벨 0/1/2/3이며, 1024² texture 기준 허용 최고 해상도는 1024²/512²/256²/128²다. 화면 footprint에 따른 자동 mip 선택은 유지한다. 텍스처별 실제 mip 수가 다르며 mip chain을 새로 생성하거나 Resources를 교체하는 작업은 아니다.

현재 `SystemOptionRows.json`에는 4개 선택지가 있지만 `CUserSettings::Apply_Video`는 이 행을 소비하지 않는다. `CShader`는 FX11 Effect와 binding 상태를 Clone 사이에 공유하고 SourceCharacter program variant를 별도 Effect로 선택한다. 표면 sampler는 현재 고정 상태다. 기존 사용자 JSON의 선택값은 0(최상)이다.

기준은 `codex/world-map-inspection-save-fix`, `69a4e48f6272eb215d7d4999543f81926b77481d`다. 같은 checkout에서 WorldSceneTool/Profiler 등 다른 세션의 미커밋 변경과 Debug Product Build가 존재한다. 제품 입력 저장은 진행 중 빌드 종료 뒤 수행하고 무관한 변경을 stage/commit/되돌리지 않는다.

## G01. 환경설정 입력과 저장

`Client/Public/UserSettingsDocument.h`에 기존 stable row ID의 이름 `TEXTURE_QUALITY`를 추가한다. `Client/Private/UserSettingsDocument.cpp::Apply_Video`는 유한한 정수 0~3을 `RENDER_QUALITY_SETTINGS::iTextureMinMip`에 전달한다. 누락/비정상 입력은 원본 품질 0으로 처리하며 다른 행은 보존한다.

`SystemOptionWindowView.cpp`의 video dirty 판정과 preset 소유 행에 texture를 포함한다. 일괄 설정은 같은 인덱스를 texture 행에 쓰며 texture를 직접 조절하면 사용자 정의가 된다. 기존 Preview → Take_VideoDirty → Activate_Profile, Cancel → 이전 snapshot Preview, Apply → 원자 저장, startup → Load_Persisted 경로를 재사용한다. 저장 schema와 Data/UI 정본을 변경하지 않는다.

## G02. Engine 품질과 실제 sampler

`Engine_RenderTypes.h`의 `iTextureMinMip`은 현재 해상도 제한 0~3을 나타내는 runtime 필드이며 기본값 0이다. 기존 `CRenderingProfileService`가 scene/region 품질에 사용자 설정을 합성한 뒤 `CRenderer`로 전달한다. 공유 RenderingProfiles JSON에는 이 개인 설정을 저장하지 않는다.

`CShader`의 현재 FX11 경로에서 명확하게 확인된 모델 표면 sampler만 대상으로 원본 descriptor와 mip 단계별 sampler를 준비한다. 주소 모드·필터·anisotropy·bias는 원본을 유지하고 MinLOD만 제한한다. 상태와 적용 단계는 Effect binding owner에 두어 Clone 사이의 캐시 불일치를 막는다. 선택된 program variant도 자신의 Begin에서 동일 정책을 적용한다. 최상으로 복귀하면 원본 상태를 복구한다.

UI/postprocess/lookup/depth/cube와 원본 Effect의 별도 sampler에는 표면 정책을 전파하지 않는다. native ModelCue helper는 AnimMesh FX의 LinearSampler를 공유하므로 `EffectModelCueNative*` 네 pass(7/8/12/13)에서 원본 품질로 복원한다. 일반 ModelCue 표면 pass0과 shadow15는 같은 사용자 품질을 적용하며 native cue는 기존 shadow 대상에서 제외된다. 각 shader의 public sampler 계약을 실제 코드로 확인한다. mip 품질에 영향을 받는 masked shadow를 캐시하는 경로는 품질 변경 시 캐시를 무효화한다. 신규 C++/HLSL 파일과 프로젝트/filter 등록은 현재 계획에 없다.

## G03. 검증과 종료

기존 UserSettingsContractHarness에서 실제 production parser/persistence/Apply_Video를 사용해 4단계, preview/cancel, 저장/reload, 실패 보존과 다른 설정 보존을 확인한다. 테스트 파일은 `out` 아래 격리하며 실제 개인 설정은 수정하지 않는다.

정상 증분 Debug Product Build로 Engine SDK와 Client까지 확인한다. 실제 CShader와 compiled FX를 사용한 격리 WARP 수치 검증으로 mip별 표면 샘플 변화, Clone/variant 적용, 최상 복원과 제외 sampler 보존을 확인한다. 빌드·자동 수치 검증·사용자 화면 판정을 RESULT에서 구분한다. 변경 문서/테스트 입력 parse와 scoped `git diff --check`를 확인한다.

사용자는 새 Client에서 ESC → 환경설정 → 비디오 → 텍스처 품질을 최상/하로 비교하고 취소·적용·재실행을 확인한다. 이 기능은 texture sampling 품질을 바꾸며 VRAM 상주량 절감, 모델 LOD/파티클 수/그림자 해상도의 신규 구현을 포함하지 않는다. 현재 팀장 조명·Bloom/FXAA/SSAO·scene/region 저장값은 보존한다.
