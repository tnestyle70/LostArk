# Rendering Workbench 촬영 흐름 결과

## G00. 구현

Workbench를 `Restoration`, `Technique A/B`, `Measure / Analyze`, `Saved Settings` 네 탭으로 분리했다.
초기 구현은 여섯 단계와 이전/다음 버튼, 원래 화면 복귀를 제공했다. 후속 G05에서14개 단계와
행별 A/B·비용 표로 확장했다. 수치44개·기법 사전·
픽셀 진단은 기본으로 접으며 저작 저장 기능은 별도 탭에 있다. 기존 ANSI TU인 MainApp의 새 탭 문구는 ASCII, 기존 UTF-8 Benchmark의 상세 설명은 한국어로 유지한다. UI 크기는 현재 ImGui frame
height와 가용 너비를 사용한다. Client를 실행해 실제 창/DPI를 확인한 것은 아니다.

시연은 기본 재질(근사) → 원본 재질 → 환경광·baked → 그림자·공간 효과 → Tone·LUT → 현재
설정이다. 모든 단계는 처음 보관한 실효 값에서 구성하고 차이 필드만 임시 소유한다. 단계
이동에서 저장된 FXAA OFF·노출·감마·원본 재질 입력을 임의로 바꾸지 않는다. 간편 A/B는
13개 기법 중 하나만 선택해 기존 recipe에 연결한다. 표준 recipe는34→35개다.

기존 Service whitelist 끝에 `material.sourceMaterials.enabled`를 추가했다. 같은 transaction의
적용·rollback·종료 복원을 사용하며 Data schema나 새 renderer owner를 만들지 않았다.
재질 OFF는 map의 기존 fallback과 diffuse가 유효한 SourceCharacter program1~5,8~17,21~32의
기존 textured deferred branch에 연결한다. hair6/7/18/19/20·forward·미확인 native program은
유지한다. 현재 WModel·텍스처·형상을 유지하므로 최초 임포트 EXE의 정확한 재현으로 표시하지 않는다.

단계에서 기법 비교로 전환할 때 흐려진 현재 화면을 A로 다시 읽지 않고 Original을 사용한다.
수동으로 새 A를 채택했던 세션이 Original로 돌아가면 experiment ID를 새로 발급해 이전 측정의
A가 새 비교의 기준으로 섞이지 않게 했다. 선택 필드만 common fingerprint에서 제외하고 full
fingerprint는 material selector를 계속 기록한다. 옛 픽셀 진단과 새 세션의 중복 owner도 거부한다.

## G01. 추가 원본 복구와 조사

실제 Character Select Movie의 WARLORD 소품·머리카락 native700/701에서 primitive opacity
prefix 누락을 수정했다. 대상·수치·shader identity의 근거는
[해당 결과](2026-10-04_MOVIE_WARLORD_HAIR_PREFIX_RESULT.md)를 따른다.

현재 native128×32 BRDF와 SH 입력 복원이 연결된 범위를 확인해 Workbench의 오래된
전면 미복구 안내를 교정했다. 프로젝트 근사와 원본 입력을 구분했다.
공식 DX11 전환은 UE4 이식 증거가 아니며, UModel export·LPK archive·ShaderMap·runtime binding
및 현재 미보유 raw 자료의 경계는 [원본 조사 결과](2026-10-04_RENDERING_SOURCE_EVIDENCE_RESULT.md)에 기록했다.

사용자가 후속으로 제시한 베른 `(98.7,49.1,-103.4)`의 녹색 지형 늘어짐과 발탄 입구 돌 표면은
별도 원인 조사 대상이다. 이 문서의 Workbench/primitive prefix 검증으로 해당 지형이 고쳐졌다고
판정하지 않는다. Bern의 밝기·GI 등 기존 튜닝은 보존했으며, 후속 요청의 기본 Fog OFF만 G04처럼 반영했다.

## G02. 검증

- 실제 Benchmark/Service 본문과 public header 기반 session 검사 **173 checks / 0 failures**.
  단계 왕복·직접 점프·한 기법 전환·원래 OFF 보존·LUT/PP 조건·실패 보존·rebase ID 분리를 확인했다.
  `out/RenderingPresentation20261004/ui/session_receipt.json`.
- 실제 Service read/validate/stage/apply/restore **144 checks / 0 failures**. renderer setter별
  적용/복원 실패와 재시도, profile/level/region/Video 변경, 비소유 필드 보존을 확인했다.
  `out/RenderingPresentation20261004/service/service-verification.json`.
- 실제 fallback policy/material bind/combat bind **953 checks / 0 failures**. 확인 program27개,
  미지원22개, diffuse override·shared shader reset·hit glow·bind failure를 확인했다.
  `out/RenderingPresentation20261004/dispatch/receipt.json`.
- 실제 condition fingerprint **73 checks / 0 failures**. 선택 material selector만 비교에서
  제외하고 비선택 debug/환경/후처리 조건은 유지했다. `out/RenderingPresentation20261004/fingerprint/run.log`.
- Benchmark, MainApp, Service, DeferredMaterialRenderUtils, MapAssetRenderUtils 총5개 변경 TU
  Debug 독립 컴파일 통과. 기존 포함 헤더의 인코딩 경고는 남아 있다.
- native700/701 WARP 검사220개와 실제 Group640 FX 컴파일 통과는 별도 prefix RESULT에 기록했다.

앞의 C++ 세션/dispatch 검사는 renderer/model/user 경계를 fixture로 사용했다. 실제 Client 외형이나
촬영 화면 검증으로 대신하지 않는다. 저장 프로필·camera·effect·composition·mapplacement의
사용자 변경을 이 작업에 섞지 않았다. 새 C++ 파일이나 project/filter 등록 변경은 없다.

## G03. 제품 반영 상태

사용자가 저장·종료한 뒤 최초 Debug Product 빌드는 PASS했다.
`out/BuildPipeline/runs/20261003T202441505Z-debug-product.json`에 기록됐다.
MainApp 새 문구의 ANSI TU 호환 수정을 포함한 두 번째 Debug 빌드는 map instance FXC 작업에서
MSB6006과 pipe EOF timeout을 기록했다. 같은 시간 다른 작업의 공통 map shader·Engine 변경이
진행 중이었으며, 오류가 그 변경의 문법 오류라는 근거는 없다.

통합 Debug Product 빌드·배포는 PASS했다.
`out/BuildPipeline/runs/20261003T211650083Z-debug-product.json`에 Engine/Shared/Server/Client와
제품 배포·필수 runtime 검증 결과가 기록됐다. Release Product도
`out/BuildPipeline/runs/20261003T214827750Z-release-product.json`에서 PASS했다.

그 사이 같은 작업 폴더에 저장된 후속 Workbench 변경을 포함한 최종 증분 Product 빌드·배포도
Debug `20261003T215100551Z-debug-product.json`, Release `20261003T215245108Z-release-product.json`
에서 모두 PASS했다. 두 영수증은 같은 `out/BuildPipeline/runs`에 있다. 최종 Release는 Client
OBJ1·EXE1·CSO0을 갱신했다. 필수 runtime 파일·Navigation 참조·Item/Valtan reward catalog
검사를 통과했다. 이 통합 빌드들은 별도 Bern 최적화와 후속 Workbench 변경도 포함하므로
이 PR만의 격리 빌드라고 기록하지 않는다. 사용자 Client/UI 실행·화면 검증은 수행하지 않았다.

## G04. Bern 기본 Fog OFF

사용자의 명시적 요청으로 Bern 기본 진입 프로필 `scene.bern.neutral-day.v1`의 `fog.enabled`를
false로 변경했다. RenderingProfiles revision은90→91이다. 원본 비교용 두 Bern 프로필과 지역
밀도·색·높이, 다른 맵·FXAA·노출·감마는 보존했다. 기존 환경 영역 갱신의 scene enable gate가
지역 안개에도 적용된다. 배 탑승 전용 presentation fog나 사용자의 이후 임시 변경은 별도 경계다.

최신 원본 hash 재확인·백업·원자적 교체 후 `Publish-RenderingProfiles.ps1 -Mode Publish` PASS.
원본/게시 JSON parse 및 의미 동치 확인, 변경은 위 bool과 revision 두 필드뿐이다.
`out/RenderingPresentation20261004/fog/receipt.json`에 source 전후 hash를 보존했다.
데이터 게시 완료이며 실행 중 메모리 Reload나 사용자 화면 확인 완료를 뜻하지 않는다.

## G05. 복원 순서와 행별 CPU/GPU 비용

발표 흐름을 `하이라이트 → 기술 및 설명 → 렌더링 → 이펙트 툴 → 렌더링 비교`로 표시하고,
Restoration을 아래14개 누적 단계의 표로 확장했다. 각 행에서 A/B 전환과 비용 측정을 실행한다.
이전/다음과 현재 완성 설정 이동, 원래 화면 복귀는 같은 세션 owner를 사용한다.

| 단계 | 비교 대상 |
|---|---|
| 01 | 현재 WModel의 기본 재질로 만든 초기 임포트 근사 |
| 02 | 지원 원본 재질, BG RNM·정적 shadow 포함 |
| 03 | MapPBR 간접 diffuse 배율과 RNM |
| 04 | MapPBR 환경 specular·project cube diffuse |
| 05 | 원본 PBR 간접광 경로와 native SH·BRDF 입력 |
| 06~08 | 방향광 그림자 → SSAO → 높이 안개 |
| 09~10 | Source tone·기본 grading → 저작 LUT·Hable 장면 탈색 |
| 11~12 | Bloom → 현재 FXAA 설정 |
| 13~14 | 원래 켜져 있던 추가 실험 SSGI → SSR·현재 전체 설정 |

03의 배율은 native 경로에서 SH·hemisphere까지 소비하지만 해당 입력 경로 자체는05에서
복귀한다. BG 전체 RNM을03에서 따로 끄는 기능은 아니다. 초기 형상·UV·COLOR0·native binding
결함은 현재 자산의 고정 복구다. 설명 패널에는07-29 WModel 연결부터10-04까지 실제 이력8개를
분리해 표시한다. 이는 날짜별 실행 파일의 재생이 아니다.

`Technique A/B`는15개 기법을 같은 순서의 목록으로 제공한다. 각 행 A는 보관한 현재 설정,
B는 한 기법만 바꾼 후보이며, 숫자로 실제 ON/OFF·기여값 방향을 표시한다. 원래 OFF를
일괄 활성화하지 않고 사용자가 선택한 비교만 적용한다. LUT 입력이나 source material 조건이
없으면 이유를 표시하고 기존 화면을 유지한다.

행 비용은 기존 Profiler 수집 경로로 A1→B1→B2→A2를 측정한다. 준비와 GPU 회수 후 각
variant 평균 CPU/GPU ms, B-A, 표본 수와 반복 CPU 차이를 같은 행에 표시한다. 첫 단계는
동일 baseline 반복이다. stage/recipe 행 ID·세션 ID·측정 ID·반복·field mask·실제 적용값과
공통 조건이 일치한 표본만 묶는다. GPU pending/invalid는 N/A, 미완료·조건 변경·과거 기준은
이유를 표시한다. 측정 완료·취소 후 선택했던 A/B로 복귀하며 원래 Profiler 수집 여부도 복원한다.
최근128개 run을 유지하고 기존 JSON에 행/측정 ID와 예상 A/B 입력을 추가했다.

같은 장면에서도 옵션 기여0이 shader 계산 생략을 뜻하지 않으며, CPU와 GPU 시간은 더하지
않는다. 단계 묶음의 관측 차이를 개별 기법의 고유 비용으로 나누지 않는다. 동적 애니메이션과
게임플레이를 고정 replay하지 않으므로 현재 측정은 탐색용이다.

사용자 확인 경로는 **Debug Client → F1 → Rendering Workbench → Restoration**이다.
같은 카메라에서 각 행의 A/B로 변화를 보고 `비용 측정`을 누른다. 기법 하나의 비용은
`Technique A/B`, sample·PCF·반경 sweep와 패스별 결과는 `Measure / Analyze`에서 확인한다.
화면 확인과 실제 장면 비용 수집은 에이전트가 실행하지 않았다. 아래 검증 기록과 구분한다.

### G05 검증

- 실제 Benchmark·ProfileService 본문을 소비하는 native 검사 **164 checks / 0 failures**.
  14단계 왕복·직접 점프·Original 복귀, source material 원래 OFF, PP OFF+보유 LUT,
  shader shadow OFF의 strength 정규화, 실패 시 이전 단계 보존을 확인했다.
  실제 Begin→Update→Finalize→Finish 흐름에 synthetic Profiler 프레임을 공급해 ABBA 종료·
  취소·Profiler 소유권 복귀, 단계/기법별 결과 보존, 다른 측정·반복·조건·적용값 거부,
  부분 GPU와 scope 누락 구분, 측정 후 외부 비소유 노출 변경 시 결과 제외를 확인했다.
  실제 Client/GPU 성능 측정은 아니다.
  증거: `out/RenderingRows20261004/receipt.json`, `probe.log`.
- 실제 writer JSON parse와44개 예상 입력, B1→A1·B2→뒤쪽 A2 연결, A의 delta=null을
  확인했다. 상세 계측의 수집 중 활성 여부만 행 표시 조건에서 정규화하며 사용자의 계측 요청과
  실제 수집 조건은 계속 검사한다. 재질 selector의 JSON bool도 preview 요청 직전 값 대신
  warmup 뒤의 실제 수집값으로 기록하며 ABBA false/true/true/false를 검증했다.
  `out/RenderingRows20261004/json-verification.json`.
- 변경 `RenderingBenchmark.cpp` 독립 Debug 컴파일 성공. 실제 Debug Product incremental
  build/deploy **PASS**, Client OBJ2·EXE1·CSO0,29.860초. 기존 포함 헤더의 인코딩 경고와
  DirectXTK PDB 경고는 남아 있다. 이번 변경은 Benchmark H/CPP이며 MainApp은 header 의존성으로
  재컴파일됐다. `out/BuildPipeline/runs/20261003T215100551Z-debug-product.json`,
  `out/RenderingRows20261004/product-debug.log`.
- 외부 설정 변경 감지와 재질 metadata 보강을 포함한 최종 Debug Product **PASS**, Client
  OBJ1·EXE1·CSO0,12.880초. `out/BuildPipeline/runs/20261003T215642085Z-debug-product.json`,
  `out/RenderingRows20261004/product-debug-metadata.log`. runtime 필수 입력 누락·무효 모두0.
- fixture 기록과 최종 소스 hash 일치, 기존 UTF-8 무BOM·CRLF 보존, `git diff --check` 성공.
  새 프로젝트 등록·shader 변경·저작/게시 데이터 변경은 없다. 이 빌드는 같은 작업 폴더의 기존
  Bern 최적화 변경을 포함하며 Workbench만의 격리 제품 빌드가 아니다. G05의 Release 재빌드와
  Client 화면 확인·실제 옵션별 ms 수집은 미실행이다.


## G06. 추가 GI 선택과 최종 Debug/Release 반영

원본 복원과 custom GI를 구분하고 SSGI full/half를 실제 renderer에 연결했다.
관련 구현·183개 WARP 검사·200개 Service/recipe/fingerprint 검사와 빌드 경계는
[SSGI 결과 G04](2026-10-04_SSGI_HALF_RESOLUTION_RESULT.md#g04-workbench-연결과-최종-빌드)에 기록했다.
최종 Debug `20261003T231005474Z-debug-product.json`과 Release
`20261003T231154821Z-release-product.json`은 모두 Product build/deploy PASS다.
영수증은 `out/BuildPipeline/runs`에 있으며 이 빌드는 같은 작업 폴더의 별도 Bern 최적화도 포함한다.
Client 실행·사용자 카메라 화면·실제 프레임 비용 측정은 수행하지 않았다.

## G07. 현재 상황에서 실행할 수 있는 A/B의 정확한 범위

2026-10-05 현재 `RenderingBenchmark.cpp`에는 빠른 기법 비교 **20개**, 상세 recipe **40개**, 누적 복원 단계
**14개**가 있다. `RenderingProfileService.h`의 실험 필드는 **49개**다. recipe ID는40개 모두 고유하고,
빠른 비교20개가 가리키는 recipe와 recipe가 가리키는 필드에 끊긴 연결은 없다. 49개 필드는 고급 수치
입력의 whitelist이며 서로 다른49개의 독립 렌더 기법이라는 뜻은 아니다.

빠른 비교는 기본/원본 재질, MapPBR 간접 diffuse, 환경 specular, 원본 PBR 간접광 경로,
normal 강도, 직접 specular, 방향광 그림자, SSAO, 높이 안개, source tone+grading, 저작 LUT,
Bloom, FXAA, SSGI, SSGI 전체/절반 해상도, SSR과 추가 texture mip·Horizon AO·SSR 교차 보정·
roughness 필터를 제공한다. 상세40개 recipe는 이 기능들의
ON/OFF·표본 수·필터·기여 강도·표시 변환을 나눈 것이다. 각 항목은 실제 Service setter와
renderer/shader 소비자에 연결되지만, 재질·자원·패스 조건에 따라 화면 차이가 없을 수 있다.
모든 객체에 동시에 적용되거나 원작의 모든 렌더링 기능이 복원됐다는 뜻으로 표시하지 않는다.

| 실제 recipe ID | 실제 소비와 영상 변화 | OFF·0 또는 낮은 값에서 생략되는 범위 |
|---|---|---|
| `material.source` | 검증된 map 및 일부 textured-deferred 캐릭터에서 기본 textured 경로와 복원 재질식을 바꾼다. 현재 WModel·텍스처를 유지하며 지원하지 않는 hair/forward 경로는 유지한다. | 다른 재질 분기를 고른다. 객체 draw·기존 텍스처·모든 native 경로를 제거하지 않는다. 최초 개발일 EXE의 재현도 아니다. |
| `pbr.directDiffuse`, `pbr.directSpecular` | 지원 MapPBR의 직접 확산/반사 성분 배율을 바꾼다. native character의 별도 직접광 식까지 일괄 변경하지 않는다. | 배율0이 광원 제출·BRDF·그림자 패스 생략을 보장하지 않는다. |
| `pbr.baked` | 지원 MapPBR의 간접 diffuse 합계에 영향을 준다. RNM뿐 아니라 현재 native SH·hemisphere·ambient 경로도 포함될 수 있다. BG RNM은 별도다. | 전체 GI OFF나 RNM 계산 비용만의 분리가 아니다. |
| `pbr.environment` | 지원 재질의 환경 cubemap specular 기여를 바꾼다. | 기존 환경 계산/객체 draw가 남을 수 있다. SSR·평면 반사·광선 추적 OFF가 아니다. |
| `pbr.cubeDiffuse` | 현재 scene RGBM cube에서 근사한 diffuse 기여다. | native source indirect가 켜진 A에서는 이 근사가 비활성이므로 recipe 준비를 거부한다. native를 자동으로 끄지 않는다. |
| `pbr.normal`, `pbr.roughness` | normal 강도와 roughness offset이 지원 표면의 조명·반사 결과를 바꾼다. | normal texture sampling이나 객체 draw를 제거하지 않는다. |
| `pbr.sourceIndirect` | material에 원본 입력이 있고 selector가 켜진 경우 원본 SH·cube·128×32 native BRDF와 관련 식을 선택한다. | OFF는 보존한 이전 환경 입력/식으로 복귀한다. 간접광 전체 제거, 캐릭터 전체 GI OFF가 아니다. |
| `shadow.pass` | 현재 방향광 caster/cache와 receiver의 동적 shadow 기여를 전환한다. | OFF이면 static/dynamic caster draw와 cache copy는 생략된다. `Render_Shadow` 호출과 depth target clear·출력복원은 남고, 재질에 이미 저장된 baked/RNM/원본 static shadow는 별도다. |
| `shadow.pcf` | receiver kernel 위치를1/9/25로 바꾼다. dynamic-baked 비교는 위치당 두 depth가 필요하다. | caster draw 수나 shadow 해상도는 줄이지 않는다. |
| `shadow.strength` | 현재 shadow 합성 강도다. | 0은 shadow pass OFF가 아니며 caster/cache 작업 제거를 보장하지 않는다. |
| `ssao.pass` | 실제 raw AO와 bilateral blur 및 적용 분기를 전환한다. | OFF이면 두 AO 패스를 생략한다. 나머지 Lights/Combined는 계속 실행하며 native/material별 AO 수신 범위는 동일하지 않다. |
| `ssao.samples`, `ssao.radius` | 실제4/8/12 표본 선택과 반경을 바꾼다. | 표본 수는 loop 작업량을 바꾸지만 PS invocation 수나 ms가 같은 비율로 변하지 않는다. 반경은 표본 수가 아니다. |
| `bloom.pass` | Bloom 필터와 최종 합성의 사용 여부를 바꾼다. | OFF이면 Bloom의 extract/filter/combine 세 패스를 생략한다. 원본 HDR·emissive와 Final 표시 패스는 남는다. |
| `bloom.intensity`, `bloom.threshold` | 번짐의 강도와 HDR 선택 기준을 바꾼다. | 강도0을 포함해 필터 패스 자체는 유지된다. |
| `fxaa.pass`, `fxaa.blend` | 기존 Final 안의 공간 AA와 subpixel blend를 바꾼다. | OFF 또는 blend0은 추가 FXAA sampling을 조기 종료한다. Final/tone 변환 패스는 생략하지 않는다. TAA/TSR/MSAA가 아니다. |
| `fog.pass`, `fog.density` | 실제 높이·지수 안개 합성이다. | OFF는 fog 계산 분기를 우회한다. Combined 자체는 남으며 volumetric raymarch/froxel 패스가 존재한다는 뜻이 아니다. |
| `display.sourcePostProcess` | source tone과 grading 묶음을 선택한다. | OFF이면 기본 Hable 표시 경로로 바뀐다. raw linear 화면이나 순수 tone-only OFF가 아니며 Final은 유지된다. |
| `display.lut` | source tone+grading이 켜진 상태에서 저작 LUT layer의 색 기여를 비교한다. | 원본 LUT 입력이 있어야 ON 가능하다. OFF도 source tone/gamma/grading 준비와 Final이 유지될 수 있다. 빈/중립 LUT에서 색 차이를 보장하지 않는다. |
| `display.exposure`, `display.gamma` | 최종 표시 변환 입력을 바꾼다. source grading cache가 준비될 수 있다. | 조명 패스를 생략하지 않는다. 첫 준비 비용과 안정 상태 비용을 구분한다. |
| `ssgi.pass` | 현재 화면 radiance에서 diffuse bounce를 찾아 기존 RNM/IBL 위에 가산하는 실제 SSGI다. | OFF이면 해당 SSGI 패스를 생략한다. SSR이 켜져 있으면 공통 HDR/bloom copy는 남고, 둘 다 OFF일 때 ScreenSpaceLighting 전체가 생략된다. |
| `ssgi.resolution` | 기존 full gather 또는 ceil-half FP16 gather + full-resolution depth/normal bilateral resolve를 선택한다. | 기본 false는 기존 full 경로다. half는 gather 픽셀 수를 줄이지만 resolve·공통 copy가 추가/유지되므로 전체 비용1/4을 보장하지 않는다. |
| `ssgi.samples`, `ssgi.radius`, `ssgi.strength` | 실제4/8/16 ray, 검색 반경, 가산 강도다. | 강도0은 ray 탐색을 조기 종료하지만 선택한 gather/resolve 또는 full pass와 공통 copy는 남는다. |
| `ssr.pass` | 현재 화면 depth/radiance에 대한 실제 specular reflection 가산이다. | OFF이면 해당 SSR 패스를 생략한다. SSGI가 켜져 있으면 공통 copy는 남는다. IBL은 유지된다. |
| `ssr.steps`, `ssr.distance`, `ssr.thickness`, `ssr.strength` | 실제16/32/64 최대 step, 거리, depth hit 허용치, 가산 강도다. | 강도0은 trace를 조기 종료하지만 패스·복사는 남는다. 두께는 실제 mesh 두께가 아니며 step 수와 실제 instruction·시간은 다르다. |

SSGI/SSR과 새 half 경로의 receiver는 **marker3 MapPBR**이다. native character marker5와
Landscape14를 포함하지 않는다. marker5의 `MaterialSpecular`는 vertex channels를 저장하므로
조건만 풀어 같은 roughness/albedo로 읽으면 잘못된 결과가 생긴다. 수신 범위를 넓히려면 해당 material의
실제 입력과 G-buffer 계약을 연결해야 한다. 반경·강도를 높여 이 누락을 복원했다고 판정하지 않는다.

## G08. 현재 A/B 세션과 비용 판정

빠른 비교는 시연 시작 때 보관한 현재 설정에서 한 항목만 바꾼 B를 준비한다. 상세 recipe도 현재 A와
연결 조건을 먼저 검사하고, 성공한 후보만 기존 B를 명시적으로 교체한다. 하위 표본 수/해상도 recipe는
A에서 상위 패스가 ON이어야 한다. 필요한 패스를 자동으로 켜지 않고 이유를 표시한다. 기법 검색·선택은
렌더링 값을 바꾸지 않는다. 임시 실험 종료 시 소유한 필드를 원래 값으로 복귀시키고 무관한 동시 튜닝은
보존한다. 저장·Publish와 session preview를 구분하며 팀장의 원래 OFF와 Mario FXAA OFF를 덮어쓰지 않는다.

촬영 단계의14개 행은 현재 구현의 누적 기여 비교다. 고정으로 복원된 UV/COLOR0/alpha/texture·CB·sampler
binding을 과거 결함 상태로 돌리는 토글은 아니다. 시작 때 꺼진 기능은 누적 시연도 그대로 꺼진 상태다.
따라서 인접 단계 A/B가 같을 수 있으며 동일 기준 반복은 측정 편차를 확인하는 용도다.

표시되는 A/B 차이는 동일 장면·카메라·해상도·노출·환경 조건의 프레임 차이다. pass 고유 비용으로
단정하지 않으며 warm-up, GPU query 회수, AB/BA 반복과 조건 일치를 확인한다. 조건이 달라졌거나
GPU 표본이 부분적이면 완전한 비교로 표시하지 않는다. PBR 기여0과 패스 OFF를 같은 비용 실험으로
해석하지 않는다. 새 half의 실제 GPU 비용은 `Render.SSGI.GatherHalf`, `Render.SSGI.ResolveHalf`,
`Render.ScreenSpaceLighting.Copy` 및 전체 frame을 함께 본다.

실행한 관련 검증은 half backend WARP183/0, 실제 UI/session recipe9/0, fingerprint90/0이다.
당시45개 필드 중 half selector만 선택하면 해당 값만 공통 조건에서 제외되고 GI ON/강도/반경/ray,
SSR·gamma·source material은 계속 비교 조건에 남는다. 픽셀 fixture의 성공을 제품 FPS나 사용자 화면
일치로 표시하지 않는다. 현재40개 recipe를 모두 제품 UI에서 클릭한 runtime smoke는 실시하지 않았다.

## G09. 설명 항목과 미구현 기능

기법 사전의 GPU Gems는 알고리즘 자료 모음이다. 연결된 SSAO 표본 실험은 설명용 기존 recipe이며
GPU Gems 전체를 켜는 기능이 아니다. Lumen·DXR/RTX·Path Tracing·DDGI/probe GI·GTAO·volumetric fog·
planar reflection·TAA/TSR·DLSS/FSR/XeSS·frame generation·Nanite·VSM 등의 행은 필요한 기반과 한계를
설명하는 항목이다. 이 audit에서 실제 pass/scene 표현/SDK backend가 확인되지 않은 것을 구현 완료로
승격하지 않는다. 현재 SSAO를 GTAO, height fog를 volumetric, SSR을 planar 또는 hardware RT로 부르지 않는다.

원본 복원과 custom quality layer도 분리한다. 원본 복원은 원본 package/build/hash, stable material,
effective static set, VF/pass/shader ID, texture/uniform binding과 실제 소비자까지 근거가 이어져야 한다.
custom SSGI half는 사용자가 허용한 자체 엔진 품질·성능 확장이며 원작에 같은 기능이 있었다는 증거가 아니다.

## G10. 2026-10-05 네 장면 촬영 목표와 현재 구현 감사

사용자의 목표는 Client EXE의 ImGui Rendering Workbench에서 before-restoration을 먼저 보여주고,
복원된 렌더링 성분과 추가 품질 옵션을 차례로 적용해 Bern·Character Select·Valtan·Kouku의
화면 변화를 촬영하는 것이다. 이번 작업은 이전 구현과 현재 코드·게시 데이터의 대조 감사다.
제품 코드·렌더링 옵션·Resources를 수정하거나 Client/UI를 실행하지 않았다.

### G10-1. 기존 요청의 반영과 연결되지 않은 촬영 흐름

08-08 HDR/Bloom Workbench, 09-06 Level/Scene 품질 저장, 09-14 Before/Restored 비교,
09-20 Mario FXAA OFF, 10-03 snapshot A/B·계측, 10-04 네 탭·14단계로 기능이 누적되어 있다.
현재 14단계·20개 빠른 비교·40개 상세 recipe·49개 실험 필드가 존재하고 ID 연결은 유효하다.
이 숫자는 독립 렌더링 레이어49개나 네 장면의 모든 객체에 대한 적용 완료를 뜻하지 않는다.

| 요청한 동작 | 실제 코드와 현재 판정 |
|---|---|
| 복원 전 화면 선택 | `Measure / Analyze → 고급: 옛 장면 프로필 비교 / 픽셀 입력 → before-restoration.v1`. 저장된 scene 조명·환경·그림자·안개·후처리 profile 비교는 연결되어 있다. 현재 shader/형상/텍스처/map-light 배치는 유지하므로 과거 EXE 전체 재현은 아니다. |
| before에서 복원 단계를 바로 진행 | 미연결. `Start_SessionExperiment`는 profile 비교 소유권이 남아 있으면 거부한다. 먼저 `Return to entry`로 돌아와야 한다. guard만 제거하면 before 설정을 최종 Original로 잘못 보관한다. |
| 복원 성분을 하나씩 누적 | `Restoration`의14개 단계가 현재 실효값 Original을 기준으로 연결되어 있다. 첫 단계는 현재 WModel의 기본 재질 근사이고 마지막 단계는 시작 때 설정이다. 원래 OFF인 기능은 계속 OFF다. |
| 추가 옵션도 연속 누적 | 개별 `Technique A/B`는 매번 Original 또는 명시적으로 선택한 품질 후보를 A로 삼는다. 앞서 눌렀던 B 위에 다음 B를 쌓지 않는다. 통합 `품질 확장 B`는 있으나 추가 기법별 누적 시연은 없다. |
| 네 Level에서 이용 | Debug F1의 Workbench는 네 Level에 연결되어 있다. Release F1에는 이 창이 없으며 F7도 제외된다. 현재 촬영 경로는 Debug Client다. |
| F1을 숨기고 같은 실험 화면 촬영 | 지원하지 않는다. F1/Workbench 숨김은 실험 종료·복원 및 Workbench capture 취소를 의미한다. ImGui를 띄운 촬영은 이 제약에 해당하지 않는다. |
| 장면 사이에서 같은 실험 유지 | Level/profile 전환 또는 camera region ID·사용자 Video 변경 시 실험이 종료된다. 각 장면과 고정 region에서 별도 시작해야 한다. |

근거는 `MainApp.cpp`의 Workbench `_DEBUG` guard와2607줄의 표시 상태 전달,
`RenderingBenchmark.cpp`의1012·1051·1183·1558·1769·1817·2292줄,
`RenderingProfileService.cpp`의827·839줄이다. 줄 번호는 감사 시점 기준이다.
구형 Saved Settings의 Live 비교는 별도 상태이므로 before profile을 촬영할 때도
`Reset comparison` 여부를 확인한다. F7의 캡처 A/B는 통계 비교이며 화면 설정을 적용하지 않는다.

프로필 닫기 실패 경로도 남아 있다. `Update_RestorationPreview`1043~1047줄은
진입 profile 복귀에 실패해도 진입 ID를 지운다. 따라서 다시 열어 수동 복귀를 재시도할 정보가
없어진다. 일반 A/B의83개 계약 검사에서는 이 함수가 fixture라 이 실패를 검증하지 않는다.
이번 감사에서 수정 완료로 표시하지 않는다.

### G10-2. 옵션 연결과 맵 재질의 실제 차이

| 게시 mapmaterials 전체 문서 | override 행 | MapPBR 정의 행 | 원본 SH/BRDF 입력 행 |
|---|---:|---:|---:|
| Character Select SL00 |280|182|181|
| Bern |23,200|1|0|
| Valtan |4,512|0|0|
| Kouku |1,734|0|0|

이 표는 전체 게시 override 문서 집계다. 현재 load scope·가시 draw·G-buffer 픽셀 수가 아니다.
기본 WModel에서 상속한 재질, 동적 Character/Deploy까지 전수 검사하지 않았으므로
맵 전체의 수신 픽셀이 반드시0이라고 단정하지 않는다. 각 경로·해시·family 분모는
`out/RenderingWorkbenchAudit20261005/material-coverage.json`에 보관했다.

SSGI·SSR과 MapPBR 성분 비교는 marker3 수신 경로에 한정된다. SL00에서는 해당 재질이
많지만 Bern/Valtan/Kouku의 주요 BG 바닥이 같은 수신 경로라는 근거는 없다. 효과가 나타나지
않는데도 패스 비용은 발생할 수 있다. 현재 recipe의 검사 대상은 global source-material과
선행 옵션이며 실제 화면의 재질·입력·수신 픽셀까지 검사하지 않는다.

SSAO/Horizon raw 패스 실행도 모든 조명 성분의 차폐를 뜻하지 않는다. MapPBR의 RNM/IBL은
RT7에서 AO 적용을 받지만 BG RNM은 RT4 성분을 AO 뒤에 더하므로 같은 차폐를 받지 않는다.
native marker5도 근사 ambient와 원본 base 조명에서 적용 범위가 다르다.

Bern neutral-day와 Valtan cool-low-key의 저장 FXAA는 enabled=true, subpixel=0이다.
`Shader_Deferred.hlsl`1785줄은 subpixel<=0일 때 필터 전체를 조기 반환하므로 bool만 A/B해도
화면은 같다. Mario의 저장 FXAA OFF를 포함해 이 값들을 자동으로 수정하지 않는다.
실효 비활성 사유 표시와 명시적 blend 실험을 분리하는 것이 후속 보완 범위다.

### G10-3. 추가 옵션과 저장값 판정

텍스처 mip, Horizon AO, SSR 교차 보정·roughness 필터, SSGI full/half는 실제
Service → Renderer → HLSL 소비자까지 연결되어 있다. 별도 묶음인 품질 확장 B도 연결되어 있다.
Horizon/SSGI/SSR 기본값은 OFF이고 현재 profile JSON에 이 추가 실험값을 영구 저장하지 않는다.
활성화 가능, 현재 저장값으로 활성화됨, 해당 재질에서 실제 화면에 기여함은 서로 다른 판정이다.

Authored와 게시 RenderingProfiles는 revision91이며 JSON 의미상 동일하다. Mario1~4가
공유하는 source-rendering profile과 region55의 FXAA OFF도 유지된다. 사용자 Video·region·
연출까지 반영한 현재 화면값은 실행 중 Workbench에서 확인해야 한다.

### G10-4. 이번에 실행한 검증과 남은 구현

현재 source에서 추출한 Service/Benchmark 계약 fixture83개와 UI 호출 위치 정적 검사6개가
모두 통과했다. recipe/field/빠른 비교 연결, JSON 동등성, Engine/Client의 두 shader source
사본 일치 등7개 inventory 검사도 통과했다. 근거는 같은 out 폴더의
`contracts/contract_receipt.json`, `contracts/static_receipt.json`, `inventory-receipt.json`이다.
fixture는 실제 renderer endpoint와 복원 profile update 일부를 대신한다. 이 통과를
before 전환·실제 ImGui 클릭·장면별 화질·GPU 시간 검증으로 확대하지 않는다.
제품 코드 변경이 없어 이번 감사에서는 Product build나 WARP 검사를 반복하지 않았다.

현재 가능한 촬영 순서는 Debug F1에서 before profile 촬영 → Return to entry →
Restoration 처음부터 시작/다음 → 현재 완성 설정 → Technique A/B 추가 품질 후보 비교다.
각 맵에서 따로 진행하고 카메라·region·해상도·노출을 고정한다. 이 흐름은 자동 연결된
하나의 before-to-restored 누적 시연이라고 설명하지 않는다.

남은 핵심은 before에서 현재 진입 profile로 안전하게 복귀하고 환경 적용 완료 뒤 시연을
시작하는 연결, 실제 재질/실효0에 따른 적용 가능 표시, 추가 품질 단계의 명시적인 누적 비교다.
profile 복귀 실패 시 수동 재시도 정보를 보존하는 처리도 필요하다. Release Workbench와
도구 숨김 중 화면 유지 기능은 현재 없으며 기존 Debug/닫기 계약 변경을 수반한다.
원본 BG/Character 전체에 SSR·SSGI 등을 확장하는 작업은 재질별 입력과 G-buffer 계약을
추가로 연결해야 하므로 checkbox만 바꿔 해결되었다고 판정하지 않는다.
