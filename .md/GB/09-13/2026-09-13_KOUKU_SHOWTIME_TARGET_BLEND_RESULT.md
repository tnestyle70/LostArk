# 쇼타임 고정·추적 조준점의 native 합성 교정

## G00. 결론과 완료 경계

추가 첨부의 조준점 본체 누락은 원본 image 파일 한 장의 부재가 아니라 **native2484 화살표 링의 RGB가 최종 합성에서 0이 되는 결함**으로 재현했다. 원본은 opacity를 RGB에 이미 곱한 뒤 A=0을 출력하지만, distortion 동반 dispatch가 일반 native의 additive coverage adapter를 생략해 공통 SrcAlpha/One 합성이 다시0을 곱했다.

설치 source의2484 dispatch와 재생성기를 수정했다. 현재 실행 중 Client/Server의 제품 EXE·CSO는 교체하지 않았다. Product 빌드와 사용자 최종 화면 확인은 아직 완료가 아니다. 앞서 반영한 도넛·공 분열·손 궤적·작은 오망성13개 문서의 설치 결과는 [패턴 결과](2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_RESULT.md)를 따른다. 계획은 [G06](2026-09-13_KOUKU_PATTERN_RADIAL_MOTION_IMPLEMENTATION_PLAN.md)이다.

## G01. 이미지와 Elements의 실제 구성

사용자 첨부 두 이미지를 직접 열람했다. 첫 이미지는 빨간 바닥 원형·삼각 화살표·교차 선, 두 번째는 흰 외곽과 주황색 내부 링이 카메라를 향하는 모습이다.

| 구분 | 원본 연결 | 현재 구성·정렬 |
|---|---|---|
| 고정 조준점 | FixedArea Projectile421991203, `par_v_rpct_realtarget_01_loc_int` | Sprite4개, EPAL_Z 활성 → Client 바닥 +Y 방향 |
| 추적 조준점 | Trace Projectile421991205, `par_v_rpct_realtarget_02_loc_int` | Sprite3개, axis-lock disabled → CAMERA_SQUARE billboard |
| 화살표 링 | `fx_g_pa_ring_arrow_01_ad`, native2484 | `fx_k_symbol_01.dds`의 삼각 화살표를 radial UV로 조합 |
| 흰색·주황색 링 | `fx_d_pa_ringmaster_01_15_tr`, native2485 | `fx_f_shine_003_2.dds`, `fx_d_line_004_1_ycl.dds`의 광선·띠를 radial UV로 조합 |

원본 texture3종은 설치 Resources에 존재하며 변환해 직접 열람했다. 완성 조준점 모양의 단독 PNG/DDS를 쓰는 구조가 아니다. `elements[].resources=[]`와 별개로 `material.sourceProfile.textures`가 native sampler를 소유한다. 이 등록 확인만으로 최종 조준점 본체 출력 성공을 선언하지 않았다.

고정형 source particle life5초, 추적형10초이며 원본 fade-in은 normalized age0→0.5 동안 증가한다. 첫 시각0의 alpha0은 원본값이다. 라이브러리 duration12/17초에는 emitter와 비어 있는 tail이 포함된다. 화면 편의를 위해 원본 fade를 상수1로 바꾸지 않았다.

## G02. 배제한 원인

원본 fixed/tracking2개와 P35가 사용하는 `.fixed.ground`, `.tracking.ground`, `.end.ground`3개 모두 실제 codec load, drawable validation, roundtrip, native resource staging, CPU 발생과 seek를 통과했다. 요소수는4/3/4/3/3이며 모두 native 입자를 생성한다.

Detail의 sprite Y 크기0.01만 보고 선으로 축소됐다고 판단하지 않았다. 실제 source StartSize는 X만 사용하며, 현재 `Resolve_ParticleSpriteScale`은 source square 또는 fixed-axis의 Y0을 X로 확장한다. t=0.5초에 고정형의 최종 정사각형은3.24~5.05m, 추적형은2.23~3.72m이며 alpha도 양수였다. 기존 코드가 처리하는 이 부분은 수정하지 않았다.

native2485는 기존 CSO·원본 DDS·ABI·색 입력으로 비영 RGB가 나왔다. material parameter·texture 순서와 dynamic 무모듈의 기본값1도 확인했다. 따라서2485나 공통 billboard 설정을 같은 수정으로 변경하지 않았다. 이 검사만으로 사용자가 본 특정 화면의 모든 가림·배치 조건까지 배제한 것은 아니다.

## G03. 확정 원인과 수정

원본2484 material의 `blendmode=blend_additive`, `opacity.expression=0`, `emissive.expression=1885`와 source PS `1f900bf8b4a67245ade80dc363d1e4e8`의 마지막 `RGB=coverage×color`, `A=0` 출력을 확인했다. 원본 엔진의 D3D blend descriptor를 직접 캡처한 증거는 없으며, source blend mode와 shader 출력 및 수치 비교를 근거로 판단했다.

제품 `Shader_EffectParticleFamilyCarrier.hlsli`의 `AdditiveOneSidedDepthRead`는 `BS_EffectAdditive`의 SrcAlpha/One을 사용한다. 일반 native dispatch는 additive·opaque·masked에 scene alpha1을 적용한다. 하지만2484의 distortion 동반 분기는 `nativeColor.a`를 그대로 반환했다. opacity가 포함된 RGB에0이 다시 곱해져 화살표 링이 사라졌다.

`Client/Bin/ShaderFiles/Shader_EffectArtistNativeDispatchKoukuNativeCases2432.hlsli`에서 **2484 case의 SceneColor alpha만1**로 연결했다. 원본 RGB·fade·PS·texture·Distortion 출력·render profile은 보존했다. 이는 원본 입자를 불투명하게 만드는 수정이 아니라, RGB에 이미 적용된 opacity를 다시 곱하지 않는 출력 변환이다.

`Tools/EffectPipeline/install_kouku_gate1_native_shaders.py`도 기존 `nativeBlend → opaque` 조건을 distortion early return에 소비하도록 수정했다.2484 번호를 하드코딩한 새 정책을 만들지 않았다. 이번 설치 patch는 검증된2484에만 적용했고 전체 native installer를 실행하거나 다른 프로그램을 광역 재생성하지 않았다.2485 translucent는 기존 alpha를 유지한다.

## G04. 수치 검증과 증거

원본 및 현재 문서 조사: `out/KoukuShowtimeTargetReview20260913/source_target_review.json`. sourcePS·색·수명·axis-lock·texture hash와 사용자의 두 이미지 열람을 기록했다. source/설치 문서는 이 조사에서 바꾸지 않았다.

실제 CPU 및 geometry scale: `out/KoukuRitualHandTrail20260913/TargetAudit/target_cpu.json`, `target_measurements.json`. 실제 입자와 최종 sprite scale을 수치로 확인했으며 Client 화면 캡처는 없다.

기존 CSO와 합성만 바꾼 비교: `out/KoukuShowtimeTargetMaterial20260913/summary.json`, `source_blend_evidence.json`, `result2432.json`, `result_one_one.json`.

| 같은 shader2432·DDS·parameter·색 입력 | 기존 SrcAlpha/One | One/One 대응 수치 |
|---|---|---|
|2484 fixed 화살표 링2개 각각 | 비영 RGB0pixels |1948pixels, maxRGB2.87511 |
|2484 tracking 화살표 링 | 비영 RGB0pixels |1904pixels, maxRGB0.191674 |
|2485 기존 링4개 | 비영 RGB 존재 | 수정 대상 아님 |

이 표의 native fixture는 원본 색과 분포 입력을 사용한 정규화된64×64 사각형이며 실제 장면·카메라의 픽셀 수가 아니다. CPU의 t=0.5초 실제 alpha·크기 검사와 같은 것으로 기록하지 않는다. 최초 재사용 probe가 잘못된 shader2304를 읽었던 `result.json`은 무효이고, 실제2432를 선택한 위 결과만 사용한다.

수정 source의 실제 Debug FXC 컴파일은 `/E main /T fx_5_0 /Zi /O1`로 성공했다. 후보는 같은 폴더의 `candidate/Shader_VtxEffectParticleKouku2432.cso`, SHA256은 `7b51ea597cde01df99d843934b86aa300a2df8e43f7d62f81c5bbfaf5c531997`이다.

`candidate_validation.json`의35개 입력 모두 통과했다.2484의15개 입력은 기존 RGB0에서 비영으로 바뀌고, 수정된 RGB 픽셀 hash는 One/One 대응과 모두 bitwise 동일했다.2485의20개 입력은 모든 render target 수치와 hash가 전후 동일했고 nonfinite는0이었다. 이 중7개 fixture는 실제 CPU t=0.5초의 alpha/dynamic 입력을 사용했으며, 고정 화살표1936pixels/maxRGB0.555855, 추적 화살표1848pixels/maxRGB0.0555855를 확인했다. RGB는 원본 상수이고 geometry는 격리 quad라는 범위를 유지한다.

제품 출력과 분리된 검증이므로 설치 Client의 적용 완료 증거로 쓰지 않는다. Python syntax와 `git diff --check`도 통과했다. 기존 설치13개 문서 hash와2485 dispatch 원문이 최종 검사에서도 유지됐다(`out/KoukuPatternMotion20260913/final_scope_checks.json`).

## G05. 남은 Product 반영

현재 Client PID45900은 `Client/Bin/Debug/Client.exe`, Server PID48036은 `Server/Bin/Debug/Server.exe`로 실행 중이다. Client/UI를 에이전트가 종료하거나 재시작하지 않았다.

정본 runner `Tools/Build/Invoke-BuildAndRegression.ps1`는 Product 단계에서 `ProductOutputGuard.psm1::Assert-StandardProductOutputsNotRunning`을 호출한다. Debug/Release Client·Server가 실행 중이면 compile과 제품 출력 교체를 거부한다. 이 보호를 우회하지 않는다. 사용자가 미저장 편집을 보존하고 두 프로그램을 종료한 뒤 다음 정상 증분 Build를 수행해야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product
```

새 제품 CSO로 사용자가 Client를 다시 연 뒤 `F1 → Effect Tool V1 → All Effects → KoukuSaydon → 쇼타임 / 장판·조준·폭발`의 고정·추적 조준점에서 확인한다. 첫0초 대신 고정형1~2.5초, 추적형2~5초의 원본 fade 구간도 확인한다. 기존 P35의 MAP 배치와 실제 추적 gameplay 연결은 이 blend patch에서 바꾸지 않았다. 사용자 화면 PASS는 아직 없다.
