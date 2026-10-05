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

## G05. 10-05 텍스처 품질과 베른 성능 재조사

현재 `c999f7e91` 기능 브랜치의 기존 미커밋 최적화를 보존하며 사용자 캡처5개와 실제 설치 입력을 읽었다. 분석은 `out/BernTexturePerformanceAudit20261005/audit.json`과 같은 폴더 `analyze.py`에 있다. Client/UI를 실행하지 않았고 Resources·개인 설정·RenderingProfiles를 변경하지 않았다.

### G05-1. 품질 설정이 줄이는 범위

`CShader::Stage_TextureQualitySamplers`는 원본 Filter·MaxAnisotropy·MipLODBias·MaxLOD를 유지하고 MinLOD만 제한한다. 최상도 화면 footprint에 따라 자동으로 작은 mip을 선택하므로 이미 mip3 이하를 쓰는 먼 표면은 하와 같을 수 있다. 낮은 등급은 draw/geometry·재질/pass 바인딩·shader의 texture 명령 수·particle simulation을 줄이지 않는다. 실제 anisotropic 필터 설정을 유지하지만 GPU 내부의 실제 fetch 수까지 같다고 측정한 것은 아니다. 모든 mip을 가진 같은 resource/SRV를 계속 사용하므로 VRAM residency를 줄이지 않으며 RNM/lightmap·lookup·후처리는 이 옵션의 대상이 아니다.

현재 설치된 `LV_BER_BERNCASTLE.mapmaterials.json` 23,200행의 typed DDS는 고유8,515개다. 표면1,580개 중 full chain1,533개, 원본 NoMipmaps 단일 mip5개, 원본 Landscape height의 부분 chain42개다. bakedLighting6,935개도 모두 full chain이다. 최근 Bern 설치 대상1,485개는 기존 설치 receipt의 SHA와 일치했다. 따라서 현재 Bern 전체가 단일 mip이라 품질 옵션이 무효라는 설명은 맞지 않는다. 이 inventory는 현재 디스크 전체 자료이며 과거 캡처의 실제 가시 텍스처나 GPU에 올라간 각 mip을 측정한 결과는 아니다.

### G05-2. 사용자 최하 품질 캡처의 잔류 비용

`하_20261004_181710_906_frame309_81064_0.json`의 export 시점 Texture.minimumMip은3이다. 프레임별 품질·실제 sampled mip은 기록하지 않으므로 모든309프레임의 품질 상태를 독립 증명하지 않는다.

| 평균 항목 | 관측 |
|---|---:|
| frame interval |47.745ms|
| CPU frame |46.943ms|
| 유효 GPU timestamp frame |47.707ms,309/309프레임|
| Map.Batch.Render CPU |15.739ms|
| 그 내부 Draw / Material / Pass CPU |10.452 /2.655 /2.214ms|
| Layer final-camera 제출 CPU |4.280ms|
| Client.Update CPU |11.343ms|
| Render.Lights GPU elapsed |0.787ms|
| main Present CPU |0.040ms|
| DXGI Local 사용량/예산 최대 비율 |23.72%|

CPU 부모/자식과 GPU 시간은 중첩되므로 합산하지 않는다. 이 캡처는 메모리 예산 부족이나 main Present 대기가 시간을 지배했다는 증거를 보이지 않는다. 낮은 품질에서도 큰 제출·갱신 비용이 남으며 MinLOD 변경은 이 비용을 제거하지 않는다. GPU timestamp는 CPU 제출 공백이 포함될 수 있는 경과 시간으로 GPU 사용률이나 단독 texture bandwidth 병목을 증명하지 않는다.

최상 설정이 기록된 병합 OFF/ON 캡처는 다른 프로세스·카메라·도구 상태이고 같은 위치의 최상/하 쌍이 아니다. 따라서 이 자료로 품질별 절약 시간을 확정하거나 최상 대비 하의 FPS 개선율을 만들지 않는다. 현재 구현의 영향 범위와 잔류 비용은 확인했지만, 모든 조건을 고정한 품질별 성능 차이는 미측정이다.

### G05-3. Directional OFF와 추가 bake의 의미

현재 `RenderingProfileService::Apply_CameraEnvironmentBase`의 LiveCompare Directional OFF는 diffuse/specular RGB만0으로 바꾼 뒤 `Add_Light`를 계속한다. ambient·source-character ambient와 RNM은 남고 `Light_Manager::Render_Lights`도 directional record를 제출한다. 이 체크박스는 직사광 기여의 시각 비교이며 조명 draw 전체를 제거하는 성능 실험이 아니다.

9/30 사용자 OFF/ON 캡처는 각각120프레임, frame69.240/68.235ms, Render.Lights GPU elapsed1.031/0.957ms다. lightDrawCalls는 양쪽의 모든 프레임에서31이고 export metadata도 같다. 당시 파일은 directional 상태 필드가 없어 이름·사용자 관찰과 실제 수치를 구분한다. 이 결과만으로 모든 조명 계산을 껐다고 해석하지 않는다. 다만 전체 light 패스 자체가 약1ms인 반면 불투명 제출·재질·갱신 비용이 훨씬 크다는 것은 관측된다.

Bern은 이미 RNM average/directional과 static shadow의 사전계산 입력을 사용한다. 현재 전체 material행 중 bakedLighting21,363행, staticShadow16,463행이며 화면 가시 비율은 아니다. 추가로 환경광만 굽는 것은 draw·재질 바인딩·geometry·이펙트 갱신을 없애지 못한다. 서로 다른 재질과 조명 입력까지 atlas/proxy로 합쳐 실제 제출과 shader를 줄이는 작업은 별도이며 이번에는 구현하지 않았다. 실제 병합 손익과 이번 불필요 청크 생성 제거는 기존 `BERN_SPATIAL_CHUNK_HLOD_RESULT.md`의 G07/G12에서 구분한다.
