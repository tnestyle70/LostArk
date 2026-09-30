# 콜로세움 재질·텍스처·조명 복원 결과

## G00. 완료 상태

2026-10-01 사용자 요청에 따라 조사 결과를 실제 Resources와 source/runtime Map 데이터에
반영했다. 추가 조사보다 현재 작업 마무리를 요청한 시점에 검증된 범위로 설치·게시했다.
전체 원본 화면 복원이 끝난 상태는 아니다. 특수 재질5종, LightFunction과 별도 carrier는
G04에 남겨 두며 사용자 화면 검증은 수행하지 않았다.

브랜치는 `codex/colosseum-material-restore-20261001`, 시작 HEAD는
`a1329e0c2ba0274f21bba74e9120eca0de26bac6`이다. 다른 채팅의 Guardian·Profiler 수정은
이 결과의 작업·검증 범위에 포함하지 않았다. 만료된 팀 LAN sync를 실행하거나
`-AllowExpired`로 우회하지 않았다.

## G01. 실제 설치와 게시

| 항목 | 반영 결과 |
|---|---|
| 물리 모델 | 기존103 + 석상·성 장식 변형3 =106 |
| 배치 | 기존1294 + 누락8 =1302 |
| named 재질 | generic139 + 기존 native45 8 + native56 2 =149slot,67 source MIC |
| 카탈로그 | 물리106 + RNM variant337 + shadow variant103 =546, 모델 파일 공유 |
| 게시 mapmaterials | 변형 포함831행 |
| 배치별 RNM | source1298 중1270 연결 |
| 배치별 static shadow | source585 중568 연결 |
| 조명 | 원본 directional1, point12, fog·source tone profile |

원본 Lightmap GUID에 이미 포함된 point light는 UNBAKED receiver로 연결했다.
directional은 RNM baked GUID count0이므로 ALL receiver를 사용한다. G8 shadow는 기존
`PROJECT_ADAPTER` penumbra `.05`, exponent2로 전달하며 원본 penumbra CPU 알고리즘과
동등하다고 기록하지 않는다. shadow atlas 또는 transfer가 다른 배치는 stable SH variant로
분리한다. shadow가 없는 배치가 다른 배치의 shadow를 상속하지 않도록 검사했다.

`scene.colosseum.source-day.v1`을 새 profile로 추가하고 LevelRegistry의 콜로세움 연결만
교체했다. 기존28개 profile·globalQuality 값은 JSON 비교로 동일함을 확인했다.
FXAA·SSAO·Bloom 등의 기존 ON/OFF와 다른 Area 튜닝을 변경하지 않았다.
source mapmaterials/maplights는 Client project/filter의 `96.DataFiles` None으로 등록했다.

공식 Map Publish와 Check, Rendering Publish가 완료됐다. 파일 반영은 실행 중 도구의
메모리 draft·Reload·이미 실행 중인 EXE의 scene descriptor를 갱신하지 않는다.
Imported의 기존 `build.receipt.json`은 최초103모델 반입의 역사적 기록이며 이번 증분 복원의
현재 상태는 이 RESULT와 아래 설치·통합 증거를 사용한다.

## G02. 텍스처와 모델 보존

- source texture178개 회수:174개는 원본 전체 mip chain,4개는 원본 자체에 mip 없음.
  생성·업샘플 mip은0개다. 설치 DDS627개에 실제 source chain을 연결했고 기존 mip0의
  크기·포맷·압축 bytes는 원본 top mip과 일치한다. PNG41개는 원본 TGA pixels와 같아
  유지하고, null PNG1개는 원본 alpha0으로만 교정했다.
- RNM DDS96개, G8 shadow DDS41개, 미연결 LightFunction 원본 texture1개를 함께 보관했다.
- 원본 metadata의 OriginalSize가 실제 저장된 top mip보다 큰 texture10개는 package에
  고해상도 pixels가 없다. 확대해 복구된 것으로 표시하지 않았다.
- native93mesh의 position/UV/basis 구조를 모두 대조했다. 기존103모델에서는 가짜 COLOR0
  63개를 제거하고 원본 color2개를 추가했으며38개는 그대로다. 새 후보3개에서는 가짜 color2개를
  제거하고1개는 그대로다. 합계67개 변경·39개 원본 bytes 유지다.
- 실제 색상은 castleflag04의1171정점과 rcarena_floor02a의442정점에 존재한다.
  native BGRA를 RGBA로 연결했다. 색이 없는 mesh는 CMesh의 기존 white default를 사용한다.
- 비색상 정점·UV·normal·tangent·index·bounds·WMAT bytes 보존 PASS. 새 export와 설치본의
  tangent W가 다른10201정점은 이번 색상 교정에서 설치본 W를 유지했다. 누락 모델의 실제
  native parallel N/T도 source proof로 보존했다.

설치 전 hash 확인, 교체 직전 재확인, 백업과 원자 교체를 사용했다. Resources와
`C:/Users/user/Desktop/GBResources2`의1109파일(411,358,322bytes)을 동일 hash로 전달했다.
그중 live 변경은1067파일이며 source복구·모델·texture·설치 receipt를 포함한다.
전달본의 `Colosseum.restore.manifest.json`에 Resources 상대 경로와 SHA-256이 있다.
전달 root의 `Map`을 팀 PC의 `Client/Bin/Resources/Map`에 같은 상대 경로로 반영한다.
코드와 Data/DataFiles는 저장소 변경을 함께 사용해야 한다.

## G03. 실행한 검증

- generic source material compiler139slot PASS.
- native45/56 재사용10slot: 원본 Base/RNM/direct shader GUID, uniform default,
  texture expression·sampler 일치 PASS. 새 native shader나 전역 shader 변경 없음.
- COLOR0 focused test6 PASS,106variant native source join 및 비색상 byte 보존 PASS.
- RNM 기존 test14, static shadow 분리 test4 PASS.
- 실제 설치/GBResources2의 색상 변경67모델, 총134파일 SHA-256 일치 PASS.
- 최종 후보 Area Validate PASS, 실제 Map Publish/Check PASS(1302placement,4 runtime files).
- 기존 rendering28profile/global 값 보존, Rendering Validate/Publish PASS.
- 변경 JSON/XML parse·재질 texture 참조 전체 존재·`git diff --check` PASS.
- 동일 VS18 Insiders의 Release `ClCompile`을 `LevelRegistry.cpp` 하나로 제한해 실제
  object 생성 PASS(exit0, 오류0, 기존 포함 header의 C4819 경고21). EXE/DLL 링크는 하지 않았다.

최초 설치 당시 표준 Release Product runner를 VS18 Insiders MSBuild로 호출했지만 실행 중인
`Client/Bin/Release/Client.exe` PID32536와 `Server/Bin/Release/Server.exe` PID32460을
ProductOutputGuard가 확인하여 컴파일 전에 차단했다. 이 시도를 빌드 성공으로 기록하지
않는다. 실행 프로세스를 임의 종료하거나 Client/UI를 실행·조작하지 않았다.
이후의 정상 Product 성공과 최신 이동 수정 후 빌드 대기는 G07에 구분한다. 사용자 화면 확인은
여전히 완료하지 않았다.

## G04. 아직 복원하지 않은 경계

| 입력 | 현재 상태 |
|---|---|
| castleflag vertical masked | exact 원본 shader 확보, VS/wind runtime adapter 미연결 |
| elevator monster material | exact 원본 확보, 해당 material adapter 미연결 |
| arena floor multilayer wet | exact 원본 확보, wet/vertex offset adapter 미연결 |
| daytime sky | exact 원본 확보, daytime/time/mirror 연결 미완료 |
| water high translucent | exact 원본 확보, scene depth/world UV와 vertex-lightmap 경로 미완료 |
| dummy helper1slot | source material identity 없음, 기존 hidden helper 유지 |
| directional LightFunction | PS60명령·VS·재질상수·texture 확보, ScreenToLight/fade/depth/attenuation RT producer 미확정 |
| ambient decal30·emitter20 | 원본 carrier 근거 확보, 신규 runtime 연결 없음 |
| DynamicLightEnvironment | 기존 EnvironmentColor×Intensity 어댑터 사용, 원본 SH 동등 복원 아님 |
| DOF/motion blur/vignette/material postprocess | 이번 적용에 포함하지 않음 |

RNM 미연결28배치와 shadow 미연결17배치는 미완료 특수 재질에 속한다.
원본 BSP7개는 hidden volume brush여서 표시 지형으로 추가하지 않았고,
EFTranslucentVolume4개를 임의 안개 renderer로 바꾸지 않았다.

## G05. 재현 증거

작업 출력은 `out/ColosseumRestore20261001/`에 보존했다.

- `materials/all.mapmaterials.json`, `native-reuse.audit.json`, `remaining-final.evidence.json`
- `geometry-colors/candidate-models.json`, `replacements.json`
- `textures/installed-texture-replacements.json`, `installed-pixel-corrections.json`,
  `source-resolution-boundaries.json`
- `lighting/scene-candidate-receipt.json`, `placement-static-shadows.json`,
  `lightfunction-native-map.json`, `lightfunction-native-programs.json`
- `integration/integration-summary.json`, `validate-final.log`, `map-publish.log`,
  `map-check.log`, `rendering-publish.log`
- `resource-install-result.json`, `data-install-result.json`, `build-product.log`
- `levelregistry-compile.log`, `levelregistry-compile-console.log`

Resources 백업은 `out/CCBack`, source 데이터 백업은 위 작업 출력의 `data-backup`에 있다.
재사용 도구는 `restore_static_source_colors.py`와
`build_map_static_shadow_variant_set.py`다. 둘 다 후보만 생성하며 직접 설치·게시하지 않는다.

## G06. 추가 요청: 레이드 기준 quality와 Bloom OFF

사용자의 명시적인 후속 튜닝 요청에 따라 최신 디스크 revision89의 콜로세움 profile을
발탄 `scene.valtan.cool-low-key.v1` quality 기준으로 정렬하고 revision90으로 게시했다.
최초 원본 복원 시점의 Bloom ON은 이 사용자 지정값으로 대체됐다.

| 항목 | 적용값 |
|---|---|
| Bloom | OFF, scene multiplier0 |
| Bloom threshold / soft knee / intensity / scatter | 1 / 0.5 / 0.8 / 1 |
| SSAO | OFF, 나머지 수치는 발탄 현재 quality와 동일 |
| Exposure / multiplier / white point / gamma | 0.73 / 1 / 1 / 2.2 |
| FXAA | 발탄과 같은 ON, 기존 콜로세움 수치 유지 |

발탄·쿠크가 공통으로 사용하는 Bloom OFF/SSAO OFF/gamma2.2를 따르며 두 reference의
서로 다른 노출·FXAA는 발탄을 기준으로 했다. 콜로세움 sourcePostProcess·LUT·조명·안개·
shadow·environment와 나머지28개 profile, globalQuality는 JSON 비교로 보존했다.
실행 중 사용자 video 설정은 기존 마지막 적용 순서를 유지한다.

후보 Validate, 최종 Publish PASS. 교체 직전 source SHA-256 재확인·백업·원자 교체를
실행했다. `out/ColosseumRestore20261001/scene-quality`에 변경 전 source/runtime,
후보, 게시 로그를 보존했다. 게시 파일과 실행 중 Workbench draft는 별도이며
현재 화면에 Reload되었다고 기록하지 않는다. 이 변경은 JSON이므로 새 shader나 resource가 없다.

## G07. 통합 전달 이력·최종 GBResources2와 빌드 경계

최초 정리 요청에 따라 기존 `GBResources2` 전달본을 실제 Client 설치본과 대조한 뒤,
동일한 콜로세움 1,109개 / 411,358,322 bytes를
`C:/Users/user/Desktop/GBResources/Map/LV_PVP_COLOSSEUM` 아래 상대 경로 그대로 추가했다.
기존 manifest, Client 설치본, GBResources2의 SHA-256과 크기가 모두 일치했으며 최종
GBResources도 1,109개 전부 일치한다. GBResources2와 Client Resources 원본은 변경하지 않았다.

같이 확인한 워터팡 Q 누락 의존 4개와 이미 동일한 WAV 3개를 포함한 이번 통합 전달 결과다.

| 항목 | 파일 수 | bytes |
|---|---:|---:|
| 신규 복사 | 1,113 | 411,961,486 |
| 동일하여 건너뜀 | 3 | 901,240 |
| 선택 전달 전체·최종 SHA-256 일치 | 1,116 | 412,862,726 |
| 상이한 기존 파일 교체/백업 | 0 | 0 |

무관한 기존 파일은 삭제하거나 덮어쓰지 않았다. Q 재사용 의존 39개도 최종 target hash를
검증했다. 설치 상대 경로와 root 이탈을 검사하고 임시 파일 검증 후 교체했다. 긴 임시파일명으로
중단된 첫 복사는 짧은 이름으로 재개했으며 최종 누락·hash 불일치는 0이다.
전체 목록과 검증 증거는 `out/GuidePersonal20261001/resource-delivery/`의
`preflight.json`, `delivery-manifest.json`, `delivery-summary.json`에 있다.

사용자의 최종 지정에 따라 실제 전달 목적지는 `C:/Users/user/Desktop/GBResources2`다.
기존 콜로세움 1,109개 / 411,358,322 bytes는 동일하여 건너뛰고, 이번 워터팡 mesh/texture
4개와 WAV 3개, 합계 7개 / 1,504,404 bytes를 추가했다. 선택한 전체 1,116개 /
412,862,726 bytes의 원본 SHA-256 일치를 다시 확인했다. 상이한 기존 파일 교체·백업은
0개이며 기존 GBResources와 원본 Client Resources는 변경하거나 삭제하지 않았다.
최종 증거는 `out/GuidePersonal20261001/resource-delivery/delivery-manifest-gbresources2.json`과
`delivery-summary-gbresources2.json`이다. Q 재사용 39개 중 추가 4개 외 35개는
이미 업로드한 GBResources 기준분과 hash가 일치한다.

팀 PC에는 기존 업로드 리소스 위에 **GBResources2**의 `Map`, `Effect`, `Sound`를
`Client/Bin/Resources` 아래 같은 상대 경로로 반영하며 코드와 Data/DataFiles는 저장소
변경을 함께 사용한다.
04:43 KST 이전 Debug·Release Product 성공은 `out/GuidePersonal20261001/product-delivery-exits.json`에
기록돼 있다. 이후 최종 이동 수정과 실패 알림 연결을 포함하는 최신 Product Debug·Release
빌드는 아직 대기 중이며, 과거 EXE 잠금 또는 이전 성공을 현재 최종 빌드 상태로 사용하지 않는다.
실제 콜로세움 재질·조명 화면 확인은 사용자가 수행한다.

## G08. 최종 재질 제품 빌드

이동 보완과 모든 최신 Client/Server 소스를 포함한 정상 Product Debug/Release가 모두 성공했다.
앞선 EXE 잠금·최종 링크/제품 빌드 대기는 해소됐으며 실행 파일과 실제 로그는
`../09-27/2026-09-27_GUIDE_AI_TOOL_IMPLEMENTATION_RESULT.md`의 G09에 기록했다.
이 결과는 기존 기능별 검증을 대체하거나 실제 Client 화면·다인 플레이·성능 확인으로 확대하지 않는다.
