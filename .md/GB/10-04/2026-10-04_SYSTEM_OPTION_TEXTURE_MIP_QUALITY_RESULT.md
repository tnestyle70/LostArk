# 환경설정 텍스처 mip 품질 연결 결과

## G00. 완료 범위

기존 환경설정 `텍스처 품질`과 `일괄 설정`을 실제 모델 표면 sampler에 연결했다. 최상/상/중/하는 최소 mip0/1/2/3으로 대응하며 화면 footprint에 따른 자동 축소 mip 선택을 유지한다. 선택 즉시 preview, 취소 snapshot 복원, 적용/확인 저장과 재실행 후 로드를 기존 개인 설정 경로로 처리한다.

Debug와 Release Product Build, 실제 제품 Engine.dll의 WARP 수치 검증, production 사용자 설정 source의 저장 검증을 완료했다. 사용자 Client/UI를 실행·조작하거나 게임 화면을 캡처하지 않았다. 사용자 장면의 최종 육안 확인은 남아 있다.

## G01. 입력부터 GPU까지 연결

`SystemOptionWindowView::Commit_Draft`는 일괄 설정0~3을 기존 stable row `combobox_texturequality`에 쓰고, texture 직접 선택은 사용자 정의4로 바꾼다. `Is_VideoRow`에 포함해 `Preview → Take_VideoDirty → MainApp Activate_Profile`이 품질을 즉시 재적용한다. Cancel/Close는 기존 snapshot을 Preview하며 Save_Draft는 기존 CUserSettings Commit과 원자 저장을 사용한다.

`CUserSettings::Apply_Video`는 finite 정수0~3만 `RENDER_QUALITY_SETTINGS::iTextureMinMip`로 변환하고 나머지는0으로 처리한다. 기존 scene/region resolver가 사용자 설정을 마지막에 합성한다. `CRenderer`는 범위를 검증한 뒤 기존 품질을 commit하고, mip 변경 시 masked static shadow cache를 무효화한다. `CGameInstance::Get_TextureMinMip`은 draw마다 LUT vector를 복사하지 않는 scalar 읽기이며 renderer 초기화 전/해제 후에는0을 반환한다.

`CShader`는 prototype 준비 중 대상 sampler의 원본 state와 MinLOD1/2/3 state를 생성한다. Filter/anisotropy/address/bias/MaxLOD는 유지한다. FX11 Effect와 Clone이 공유하는 EFFECT_BINDINGS가 state와 마지막 적용 단계를 함께 소유한다. 실제 source program variant의 Begin도 동일 정책을 적용하며, 상태 변경 실패는 draw 전에 이전 sampler로 되돌린다. 최상은 원본 sampler 자체를 사용한다.

대상은 MaterialAnisotropicSampler, SurfaceAnisotropicSampler와 Mirror3종, SourceCharacterSampler/StampSampler, SourceMapMonsterStateSampler, SourceMapSkyCloudSampler와 surface contract를 가진 FX의 LinearSampler다. 맵 인스턴스의 일반 재질도 이 경로로 적용된다. SurfaceLightmapSampler와 SourceCharacterLookupSampler는 BRDF/roughness cube 등과 공유하므로 제외한다. UI/Deferred 공통 LinearSampler와 depth/후처리 sampler도 보존한다.

AnimMesh의 native Effect helper는 LinearSampler를 공유하므로 `EffectModelCueNative*` 네 pass(7/8/12/13)는 원본 sampler를 사용한다. 일반 모델 cue의 color0 및 shadow15, 캐릭터 cutin5는 사용자 품질을 함께 적용한다. native cue는 기존 shadow 대상에서 제외된다. 같은 Clone owner에서 native→surface를 오가도 다음 Begin이 올바른 상태를 재적용한다.

제품 변경은 기존 Engine6개, Client3개 파일이다. 신규 제품 C++/HLSL 파일·project/filter 등록·JSON schema·publisher 변경은 없다. 사용자 UserSettings.json, RenderingProfiles 저작/게시 파일과 Resources를 수정하지 않았다. CLAUDE의 사용법 및 gotchas/렌더링이펙트복원V2의 반복 방지 원리를 갱신했다.

## G02. 실제 검증

| 실행 | 결과와 확인 범위 |
|---|---|
| `Tools/UserSettingsContractHarness/Run-UserSettingsContractHarness.ps1 -OutputDirectory out/UserSettingsContractHarnessTextureMip` | production UserSettingsDocument/DataJson 컴파일·실행 exit0, 117 assertions PASS |
| 설정 검증 | 4단계 실효값, Preview 파일 불변, Cancel 복원, Commit/new instance reload, 잘못된 값, 기존 gamma/Bloom/FXAA/SSAO/color filter 보존 |
| Debug Product Build | 첫 빌드144.8초 PASS; public header 반영으로 Engine37/Client226 OBJ, CSO0. native scope 보완 후 최종10.7초 PASS, Shader OBJ1 및 Engine.dll 배포만 갱신 |
| Release Product Build | 같은 checkout의 선행 Release build 이후 최종 보완12.9초 PASS, Shader OBJ1·Engine.dll 갱신/배포, Client OBJ0, CSO0 |
| 최종 실제 Engine.dll WARP probe | 16fixtures, sampler descriptor795회, 실제 bound sampler의 합성 mip 수치400회, clone Begin160회, 제외 상태 보존420회 PASS |
| GPU 연결 | 실제 Initialize_Engine(WARP) → Apply_RenderQualitySettings → CShader::Begin, mip0→3→1→2→0, map/anim/instance/base 및 SourceGroup/light/UI/final 확인 |
| native 경계 | native7/8/12/13 원본 유지, surface/shadow/cutin 품질 변경, 같은 공유 Clone의 surface→native→shadow→native→surface 재적용 PASS |
| 오류/수명 | renderer 미초기화/해제 getter0, mip4 거부와 이전 품질 보존 PASS |
| 배포 | Debug/Release 각각 Engine/Bin과 Client/Bin의 Engine.dll SHA256 동일 |
| 구조·diff | 기존 옵션 JSON 및 검증 receipt JSON parse, 변경 범위 `git diff --check` PASS; 변경한 JSON/XML 파일 없음 |

GPU probe는 hidden test HWND를 사용해 Engine만 초기화했다. ShowWindow/Client/게임 장면/Present/화면 캡처를 호출하지 않았다. 실제 bound sampler를 격리 합성 texture에 적용한 숫자 readback이며 최종 장면 fidelity, FPS 향상, 메모리 절감 판정이 아니다. portrait는 동일한 캐릭터 표면 경로까지 확인했고 실제 portrait 요청/캐릭터 화면은 실행하지 않았다.

빌드는 기존 문자 집합 C4819/C4828 및 외부 PDB LNK4099 경고가 남지만 오류0이다. 기존 파일 encoding/BOM/줄바꿈은 유지했다. Product의 기본 runtime file/nav/reward 검사와 실제 전체 gameplay 검증을 구분하며 변경 없는 data를 재게시하지 않았다.

## G03. 증거와 빌드 결과

- 설정: `out/UserSettingsContractHarnessTextureMip/{results.txt,receipt.json,fixtures}`.
- GPU: `out/SystemOptionTextureMipQuality20261004/{TextureMipQualityProbe.cpp,Run-TextureMipQualityProbe.ps1,probe.log,probe-result.json,probe-inputs.json}`. 초기8fixture 기록은 `initial-eight-fixtures-*`에 보존한다.
- 빌드: `out/BuildPipeline/runs/20261004T043039244Z-debug-product.json`, `20261004T043423820Z-debug-product.json`, `20261004T043511416Z-release-product.json`. 해당 task out의 build/final-build/final-release-build에 상세 로그가 있다.
- 최종 Debug Engine.dll SHA256: `614611953851f0e9e2f726289cd2c24f9601165498a3e8890fd6e1be40cfd873`.
- 최종 Release Engine.dll SHA256: `7a5860dc9302e5f42fa2b95c2d97278cf3bae9468d58e4bdfa841def7a88e7e9`.
- 제품 실행 파일: `Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe`. 실행 작업 디렉터리는 기존 `Client/Default`다.

빌드/격리 probe/backup은 Git 산출물에서 제외한다. 같은 checkout의 다른 세션 dirty 변경을 보존했으며 자동 stage/commit/push하지 않았다.

## G04. 사용자 확인과 남은 경계

새 Client에서 `ESC → 환경설정 → 비디오 → 텍스처 품질`을 최상/하로 바꾸어 같은 가까운 맵·캐릭터 표면을 비교한다. 취소 후 이전 상태 복원, 적용/확인 뒤 재실행 유지와 일괄 설정을 확인한다. 현재 저작 조명/후처리 튜닝값은 보존된다.

단일 mip texture는 하위 단계가 없어 선택 차이가 없을 수 있다. 모든 texture가11mip인 것은 아니다. 이번 변경은 기존 mip 샘플링 품질이며 VRAM residency/streaming, 모델 LOD, 파티클 수, 그림자 해상도를 새로 구현한 결과가 아니다. 사용자 최종 화면 확인은 미실행으로 남긴다.
