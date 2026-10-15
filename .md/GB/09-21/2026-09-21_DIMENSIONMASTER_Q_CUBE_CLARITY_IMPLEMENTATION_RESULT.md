# 차원술사 Q·R·V 재질 적응과 T 복구 구현 결과

## G00. 현재 상태와 사용자 판정

최종 사용자 지시는 현재 렌더링을 올바른 기준으로 고정하고 Q 황금 큐브, R 검격 누락,
V 시작의 큰 하늘색 유리 표현을 고치는 것이다. 정상인 ALT V와 A 검격을 보존한다.
G07의 맵 grading 조사는 원인 구분 자료이며 전체 이펙트의 독립 합성 구현 지시로 사용하지
않는다. 이번 후속은 각 스킬의 실제 재질·재생 입력에 한정한다.

사용자는 1차 제품을 직접 확인한 뒤 “처음의 밝은 황금색 느낌이 전혀 나지 않는다”고
보고했다. 따라서 G01~G03의 첫 적용은 구현·빌드만 통과했고 시각 목표는 미달이다.
G04 이후는 원본 Character Select와 실제 Q 색을 다시 조사한 후속 수정이다. 현재 최종 Q
shader와 V 세 유리의 투명도 조정, T 소환체 환경 조명 수신 복구를 반영했다. 검증용
Product Debug 증분 빌드·배포는11:04 KST에 통과했으며 최종 결과는 G11에 기록한다. Client/Server는
빌드 시작 시 실행 중이지 않았다. 전역 rendering/Bloom은 변경하지 않았다.

사용자가 이어 제기한 쿠크 3관문의 보라색→파란색 문제는 G07에서 별도로 조사했다.
맵 LUT의 색 회전을 GPU로 재현했으며, 현재 Q 재질 보정에는 맵 후처리에서 자체 색을
분리하는 기능이 없다. 전체 스킬 색 독립 처리는 사용자 후속 지시에 따라 구현하지 않는다.
R 검격은 원인 미확정이며 제품 변경이 없다. T 전체 이펙트 복원은 원본 stage 구성을
대조 중이고, 이번 우선 빌드에는 확인된 환경 조명 경로 복구만 포함한다.

### 1차 반영 기록

Q의 큐브 재질 계산을 보정하는 소스와 두 저작 문서를 반영했다. 사용자가 Q 저장 및
Client/Server 종료 후 반영을 승인했고, 최신 디스크를 재확인해 두 파일을 원자적으로
교체했다. Product Debug 증분 빌드와 정상 배포까지 통과했다. 실제 화면의 미적 판정은
사용자가 수행한다.

대상은 `effect.dimensionmaster.skill.2050100.restore`와 실제 Q animevent/catalog가 연결한
`effect.dimensionmaster.skill.2050100.tuning.restore`의
`authored.source-particle.b4984eeabefb0fc93783ecda` 하나씩이다. 선택적
`project_clarity_strength=1`만 추가한다. 비교본의 기존 `bloomIntensity=16`, 요소 개수2/5와
큐브 개수·크기·배치·수명·속도·기존 재질 값은 보존한다. full.restore, 다른 스킬과 전역
Bloom·노출·조명·tone·LUT는 이번 수정 대상이 아니다. 다른 세션의 dirty 변경은 유지한다.

## G01. 코드 반영

| 파일 | 실제 변경 |
|---|---|
| `Client/Private/Effect_DocumentCodec_MaterialValidation.cpp` | named scalar의 유한성·중복 검사에 이어 CubeSample의 project clarity를0~1로 제한; Parse/Save에 같은 검증 적용 |
| `Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp` | 기존 SourceScalars0.w에 기본0으로 stage하고 GPU 준비 전 범위 재검증 |
| `Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli` | 밝은 장면의 본체·caustic highlight 압축, 원래 edge 유지, 중간 alpha coverage 보정 |

새 struct/CBuffer/profile/프로젝트 파일은 없다. 기존 scalar 편집·직렬화·staging signature와
shader binding을 사용한다. 기존 파일의 UTF-8 BOM 없음과 CRLF를 유지했다.

실제 배경 max RGB0.15~0.8에서 본체 보정을 켜며 본체 peak0.35 이상의 shoulder는
`0.65+0.45*facing`, caustic은0.75에 접근한다. 공통 RGB gain으로 색 비율을 유지하고
모서리 항은 그대로 남긴다. 배경0.5~4에서는 `a+w*a*(1-a)`로 중간 alpha를 강화한다.
fade0/1과 원래 clip 판정은 유지한다. 실제 SceneHDR로 결정한 gain·alpha는 기존 Bloom/black
보조 평가에서도 고정해 억제된 SceneBloom을 다시 만들지 않는다. 강도0은 기존 출력,
강도1의 보정분은 SceneBloom을 선형으로 운반한다. 이는 PROJECT_TUNED이며 원본 식과의
동일성을 주장하지 않는다.

## G02. 실행한 검증

- 수정 ResourceStaging CPP의 독립 컴파일 통과. 현재 MaterialValidation을 포함한 production
  Codec TU와 material support를 현재 헤더로 다시 컴파일했다.
- 최종 include를 사용하는 `Shader_VtxEffectMeshPreview.hlsl` 전체를 fxc fx_5_0으로 컴파일해
  out CSO 생성을 확인했다. Effects deprecated 및 기존 공통 shader의 uninitialized/isfinite
  경고가 있고 error는 없다. 제품 설치 경로의 CSO를 교체한 결과는 아니다.
- 이어 정상 `Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`가
  Engine→Shared→Server→Client 전체 PASS, exit0으로 끝났다. 소요351816ms이며 Client OBJ88,
  CSO1 및 Client.exe를 갱신했다. 다른 기존 dirty 변경도 포함한 정상 증분 빌드이며
  Clean/Rebuild나 임의 CSO 복사는 하지 않았다. 기존 EngineSDK Level.h의 C4828 경고와
  공통 shader 경고가 있으며 오류는 없다.
- 실제 Codec 검사65개 통과: 기본값 생략,0/0.5/1의 Parse·Validate_Drawable·Save_Atomic·재Load,
  정확한 roundtrip, 음수/1초과 거절과 실패 시 기존 문서 보존, NaN/Inf/중복 거절,
  scalar 추가·변경의 staging signature 변화, 실제 두 후보의 load/drawable/roundtrip.
- 실제 production CubeSample include와 SceneColor hook의 D3D11 WARP readback1455조건 통과.
  opt-off 최대 상대 오차0, 어두운 배경≤0.15의 normal 출력 오차1.05e-7,
  strength1 SceneBloom 운반 선형성 오차9.31e-10. alpha 끝값·clip·coverage 범위,
  zero bloom/black 일치, 본체 상한·색 비율을 검사했다.
- HDR60000 정면에서 큰 legacy 값의 lerp 상쇄를 발견해 보정 transmission을 가중합으로
  수정했다. 최종 shader hash는 `d6bd3069640261e222c9e5cc8f501c3c334c5092069e889128fc2a7411059fd3`다.
- 각 후보에서 새 scalar를 제거한 구조가 최신 원본과 같음을 확인했다. 승인 후 교체한
  저작 원본의 hash도 검증한 두 후보와 일치한다.
- `git diff --check` 통과. 전체 `Validate-EffectSources.ps1 -ResourceRoot Client/Bin/Resources
  -AllowLocalResources`는 수정하지 않은 기존 `effect.kouku.gate1.blade-dance.circle.impact`의
  `v15 Product document has no runtime carriers`로 실패했다. 해당 파일은 Git 변경이 없으며
  이번 Q 후보의 Codec 검사는 별도로 통과했다. 전역 validator 실패를 숨기거나 쿠크 문서를
  이번 요청에 포함해 수정하지 않았다.

GPU readback에 alpha 합성과 Character Select neutral source tone 식을 적용한 표본:

| 배경 RGB | facing | 기존 edge−body | 적용 후 |
|---|---:|---:|---:|
| `[1,1,1]` |0.5|0.00212|0.05159|
| `[4,4,4]` |0.5|0|0.02880|
| `[16,16,16]` |0.5|0|0.00498|

배경0.1의 대비는 유지된다. 극단적으로 밝은 배경에서는 적용 후에도 대비가 작아지는 한계가
있다. 위 검사는 합성 텍스처·RGBA32 1픽셀과 CPU tone 계산이며 실제 맵·DDS·Client 전체
합성 화면의 승인이나 모든 최종 픽셀 동일성 검사가 아니다. Codec 검사의 playback validator는
production 함수 본문을 그대로 추출해 링크했으며 실제 GPU ResourceStaging 전체 실행은 아니다.

증거는 `out/DimensionMasterQClarity20260921/`의 `codec/result.json`, `codec/inputs.json`,
`gpu/summary.json`, `gpu/tested-inputs.json`, `gpu/readback.csv`, `gpu/composite-contrast.csv`,
`candidate-receipt.json`에 있다. 이 로컬 검증 산출물은 소스 커밋에 포함하지 않는다.
제품 빌드 증거는 `out/BuildPipeline/runs/20260920T232853078Z-debug-product.json`, 상세 로그는
`out/DimensionMasterQClarity20260921/product-build/`다. 설치된
`Client/Bin/Debug/Shader_VtxEffectMeshPreview.cso`의 SHA256은
`ec772c5e66a28d470591612e7482d501a8b1c643bab514ef8a79b547c77a5f3c`이며 Client.exe는
2026-09-21 08:28:51 KST에 갱신됐다. 빌드 후 JSON과 `git diff --check`도 통과했다.

## G03. 저장본 반영과 사용자 확인

사용자 답변은 “Q 편집 저장·Client/Server 종료 완료, 반영 진행”이다. 최신 파일을 다시 읽어
stable ID/scalar만 병합했고 교체 직전 hash 확인, backup, Windows ReplaceFileW 원자적 교체,
교체된 원본 backup과 예상 원본의 동일성 및 설치 hash를 확인했다. restore의 최종 hash는
`024281ce7b14a404a3664f68494450d4e8f21df1fc332762b329940cdd2b83a6`, tuning은
`558441926c2ddc2c5a0579fc10114c8b744d62171d4cc71e9c519674016db8dd`다.
영수증과 직전 원본은 `out/DimensionMasterQClarity20260921/apply-20260920T232254811282Z/`에
보존했다. Client/Server 종료를 확인한 뒤 정상 Product Debug 증분 빌드를 완료했다.
독립 최종 데이터 검토에서도 설치본/후보의 byte 동일성, stable cube 외 모든 필드 보존,
Authored 전체에서 새 scalar가 Q 두 문서에만 존재함과 animevent/catalog의 tuning 소비를 확인했다.

제품 실행 뒤 F1 → Effect Tool V1의 `이펙트_차원술사Q`/`이펙트_차원술사Q_튜닝`을 열고,
실제 Q는 Lobby → Character Select → 차원술사에서 사용자가 확인한다. Bern/쿠크에서도 같은
Q의 면·모서리·투명 fade를 비교한다. 외부 파일 교체는 실행 중 메모리 draft를 Reload한 것이
아니며 사용자 편집을 버리는 Reload·종료·UI 조작을 자동으로 수행하지 않는다.

완료 시 Server CMD와 Client는 모두 종료 상태다. 사용자가
`Server/Bin/Debug/Server.exe`를 실행한 뒤 `Client/Bin/Debug/Client.exe`를 실행하면 방금 빌드한
제품으로 확인할 수 있다. F5/Ctrl+F5를 무빌드 실행으로 안내하지 않는다. 본 작업은 제품을
자율 실행하거나 화면을 캡처하지 않았으며 최초 깔끔한 표현과의 최종 시각 일치는 미확인이다.

## G04. 원본 Character Select와 Q 재검토

원본 `BG_PCSELECT02` selected source의 CharacterCloseup Uber export1736은 customizable
tone/toe0.5와 World settings 상속을 사용한다. 활성 원본 handoff의 toneScale1/range8,
중성 grading/desaturation0과 현재 Character Select가 맞는다. 현재 profile의 Bloom off는
원본 on과 다르지만 사용자가 on도 시험했으므로 이것만으로 면 발광 부재를 설명할 수 없다.
별도 source volume151에 금빛 Bloom tint/LUT가 있으나 중앙 아레나에서 약2km 떨어진
영역이므로 이를 전역 복원할 근거가 없다.

Directional comparison off는 diffuse/specular만 제거하고 baked RNM/IBL/ambient와 tone을
유지한다. LUT off는 LutLayers만 비우며 현재 Character Select는 원래 빈 LUT다. 따라서
두 옵션 off는 예전 Hable 렌더링이나 과거의 SceneHDR를 복원하는 실험이 아니다.

실제 Q의 HDR particle RGB는 `(7,6,1)`, alpha는0.6, MIC color는 `(4,3.8,3,1)`이다.
ColorScaleOverLife 분포 prefix를 제외해 해석한 RGB는 Playback→Particles→MaterialBinding→
Mesh shader까지 float4로 전달되며 UNORM/clamp가 없다. 원본 shader는 선명한 particle색을
edge에만 사용한다. 넓은 면은 background×warm tint이며 1차 수정은 이를0.65~1.1로 눌렀다.

Mesh carrier는 `F(SceneBloom)-F(black)+Write_SceneBloom(F(black))`로 합성한다. 1차 수정의
배경 본체는 F(black)에서0이고 정면 center는 caustic/edge도0에 가까워 Bloom16을 곱해도
면 발광이 생기지 않는다. 도화가·창술사의 texture/particle 자체 발광과 다른 점이다.

원본 자료는 `out/RenderingRestoration20260914/BG_PCSELECT02.selected-source.json` 및
`rendering_source_handoff.json`, 초기 노란 큐브 사용자 관찰은
`09-08/2026-09-08_CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT.md`에 있다.
원본 자료·현재 코드의 소비와 사용자의 실제 화면 판정은 구분한다.

## G05. 황금색 자체 발광 후속 수정

이번 후속의 제품 소스 변경은 `Shader_EffectCubeSampleScene.hlsli` 하나다. 앞 단계에 저장한
Q 두 문서의 clarity1을 그대로 소비한다. 다른 동일 재질은 기본0으로 원래 RGB/alpha를 유지한다.

배경 transmission만 제한하고 particle의 정규화 chroma `(1,6/7,1/7)`로 금색 흡수를 만든다.
별도 면 발광은 `particleRGB*chroma*MIC.rgb*MIC.a*emissive*(.25+.15*grazing²)`이다.
정면 `(7,4.885714,.107143)`, 측면 `(11.2,7.817143,.171429)`의 자체 HDR을 배경 제한 밖에
더한다. caustic0.75 제한은 제거하고 원래 edge/caustic을 유지한다. alpha는 원본0.6에서
밝은 장면에 한해 최대0.84까지 보강하며 fade0/1과 원본 clip 판정은 유지한다.

광도만 더한 내부 후보는 수치 안전성2571조건을 통과했지만 source tone 후 회색 배경1의
정면 B/R이0.890, 배경4에서0.951로 여전히 너무 옅었다. 이를 완료로 처리하지 않고 chroma
흡수와 자체 발광의 채도를 보강했다. alpha0.6에서는 배경4의 파랑 잔여만1.6이므로 실제
발광과 함께 배경 누출을 줄여야 색이 남는다. 해당 내부 후보 산출물은 out의 variant로 보존한다.

자체 발광은 scene read mode0/1/2에서 동일하며 기존 Bloom 분리식을 유지한다. Bloom이
꺼져도 SceneColor에 금색이 남고, 켜면 기존 개별 intensity가 자체 발광에 작용한다. 배경의
억제된 Bloom을 재생성하지 않는다. 새 face/chroma 계수는 PROJECT_TUNED이며 원본 shader
그대로의 복원이라고 주장하지 않는다.

후속 shader SHA256은 `dec7d7d676092e5ba6a108302da64c7efcc335275189ad32367c2a0a471282c9`다.
수정 전1차 shader와 Q JSON은 `out/DimensionMasterQGold20260921/baseline/`에 보존했다.
최종 사용자 화면은 미확인이다.

## G06. 실제 Q 입력과 현재 Character Select tone 검증

최종 DEC7 shader의 D3D11 WARP 검사2580조건이 실패0으로 끝났다. production CubeSample,
scene hook, Extract_SceneBloom/Write_SceneBloom, source tone 함수를 컴파일했다. 설치된
원본 caustic DDS를 sRGB/wrap으로 읽고 실제 particle `(7,6,1,.6)`과 face/edge UV를 사용했다.
opt-off 원본 일치 오차0, SceneBloom 선형 운반 최대 상대 오차1.91e-7, 자체 emission RGB의
scene 의존 오차0이다. alpha끝값/clip/frozen mode와 범위, intensity0/1.3/16의 SceneColor
불변 및 자체 Bloom 응답, SceneBloom0에서 면 발광/Bloom 양수를 확인했다.

정면 center의 alpha 합성 후 실제 CS tone 수치(Bloom blur를 더하기 전):

| 회색 SceneHDR | 후속 display RGB | 1차 R−B | 후속 R−B |
|---:|---|---:|---:|
|0.1|(.9794,.9608,.3886)|.0673|.5908|
|0.5|(.9860,.9713,.6324)|.0361|.3535|
|1|(.9864,.9722,.7205)|.0266|.2659|
|4|(.9979,.9878,.8114)|.0188|.1865|
|16|(1,.9979,.9513)|—|.0487|

정면/중간/옆면 모두 배경0.5/1의 R−B≥0.2, 배경4의≥0.12를 통과했다. 극HDR16에서는
tone 포화로 차이가 작아진다. 숫자는 native CS packing `(.22,1.0275,.25544716,1)`과
toe0.5의 production tone까지 포함하지만, neutral LUT 양자화·최종 Bloom blur·실제 맵
렌더링 전체와 사용자 미적 판정은 포함하지 않는다. 기존 비교본의 Bloom16을 보존했으므로
Bloom on의 겹친 밝은 면/퍼짐은 별도 사용자 확인 항목이다. 전역 Bloom 값을 바꾸지 않았다.

증거는 `out/DimensionMasterQGold20260921/gpu/summary.json`, `readback.csv`,
`gold-tone.csv`, `three-version-body.csv`다. 직전 옅은 금색 후보의 산출물은
`gpu/variant-initial-gold-4CC317C6/`에 보존했다. `git diff --check`도 통과했다.

이어 DEC7 소스를 포함한 전체 `Shader_VtxEffectMeshPreview.hlsl`도 fxc fx_5_0 컴파일이
exit0으로 통과했다. 출력과 로그는 `out/DimensionMasterQGold20260921/`의
`Shader_VtxEffectMeshPreview.cso`, `Shader_VtxEffectMeshPreview.compile.log`다.
기존 공통 shader의 potentially-uninitialized 경고 등이 있고 오류는 없다. 이 out CSO는
제품 경로로 복사하지 않았으며 정상 Product 빌드 완료를 뜻하지 않는다.

현재 Client PID79584와 Server PID74576가 제품 출력을 점유해 정상 Product 빌드는 종료 후
수행한다. 이번 후속에서 Q JSON은 byte 단위로 변경하지 않았으며 데이터 교체 승인/Reload는
요청하지 않는다. 최종 제품 빌드 결과와 실행 준비 상태는 완료 후 이 절에 기록한다.

## G07. 맵 색 보정과 차원술사 전체 소비 경로 조사

현재 skillbinding→animevent가 사용하는 14개 문서는 총714요소이며 visible711이다.
visible 중73개는 live SceneColor를 직접 읽고,622개는 자체 RGB/texture/mask,14개는
light,1개는 시작 화면 capture operation,1개는 조사상 미확정 generic particle이다.
마지막 항목은 F의 `authored.source-particle.full-f.63acbc26559dd742ea96`이며 비활성
sourceProfile/빈 resource로 inventory에서 분류하지 못했을 뿐 runtime 미지원 판정이 아니다.
ALT_V의 비활성 frozen capture mesh2개와 별도 animated ModelCue,
T의 별도 CModel summon은 이 직접 SceneColor 통계와 구분한다. 조사표와 stable asset 경로는
`out/DimensionMasterSceneDependencyAudit20260921/inventory.json` 및 `SUMMARY.md`에 있다.
실제 mesh sourceMaterialSlots도 반영했고18개 scene-reader family는 descriptor와 선택된
HLSL 함수의 Read_EffectSceneColor/Bias 호출을 대조했다. CubeSample은 Q1/R1/A4의6개이며
나머지67개 reader를 큐브와 같은 식으로 취급하지 않는다.

자체 RGB 요소도 배경과 SceneHDR에 합쳐진 뒤 최종 tone/LUT를 통과한다. 도화가·창술사도
같은 경로이며, 조명 비의존(unlit)이나 per-effect Bloom의 분리가 맵 색 보정 제외를 뜻하지
않는다. 현재 V1 3D effect에는 개별 tone/LUT bypass가 없다. V2의 bDisplaySpace 화면
overlay는 depth·굴절·Bloom을 사용하는 3D 재질의 대체 경로가 아니다.

현재 설치된 쿠크 G3의 `kouku.ps.environment.48` 완전 적용 상태를 대상으로 production
source tone, BGRA8 LUT bake, LUT lookup을 D3D11 WARP에서 실행했다. 노출은 CS/G3 모두1,
CS toe0.5와 중성 grading, G3 toe0.8/desaturation0.125/LUT01을 사용했다. 실제 카메라의
region 선택이나1초 전환 중간 상태를 관찰한 것은 아니다.

현재 purple-rim shader의 `(.48,.12,.90)`을 배경·Bloom 없이 고정 HDR 입력으로 넣은 결과:

| 처리 | 최종 RGB | 색상각 |
|---|---|---:|
| Character Select 중성 처리 |(.704687,.372120,.825670)|283.995°|
| G3 LUT01 + desaturation0.125 |(.271858,.541973,.715555)|203.473°|
| G3 LUT OFF, tone/desaturation 유지 |(.681170,.385589,.787601)|284.115°|
| G3 LUT01 유지, desaturation OFF |(.241624,.551105,.749160)|203.414°|

15개 RGB 입력×4경로의60개 표본이 유한하며, 이 보라→청색 회전은 LUT01만으로 재현된다.
표본 `(.48,.12,.90)`은 R/A의 PROJECT_TUNED purple-rim 값이고 원본 추출색이 아니다.
R/A LocalCrack의 보라색 MIC도 기존 사용자 조정이므로 원본 복원을 이유로 자동 폐기하지 않는다.
CubeSample tint나 particle색을 직접 넣은 나머지 표본 역시 완성된 유리 pixel을 뜻하지 않는다.

LUT01은 원본 추출 receipt와 독립 UModel 대조4096픽셀의 mismatch0 근거가 있다. 따라서
파랗게 나온다는 이유만으로 잘못된 LUT라거나 원본 재질을 다시 읽으면 해결된다고 단정하지
않는다. 원본 재질 계산 복원과 맵 grading으로부터 자체 색을 보호하는 것은 서로 다른 변경이다.
현재 확보된 원본 재질 evidence에서도 unlit/translucent/additive는 확인되지만 별도 tone/LUT
우회 근거는 확인하지 못했다. 선택적 독립 색 처리는 프로젝트 표현 정책으로 구분해야 한다.

현재 코드에서 가능한 분리 지점은 기존 renderer 안의 합성 경계다. 재질의 자체 색·모서리·발광은
중성 색 처리 대상으로 보존하고, SceneColor 투과·굴절은 배경을 계속 소비해야 원본 유리 성질이
남는다. 이미 섞인 SceneHDR pixel 전체에 stencil로 LUT를 끄면 그 안의 배경까지 바뀐다.
별도 색 출력으로 확장할 경우 depth, 반투명 정렬·앞쪽 투명체 감쇠, distortion, 자체 Bloom과
이중 합산 방지를 함께 닫아야 한다. 이 합성 확장과 전스킬 적용은 이번 조사에서 구현하지 않았다.

GPU 근거는 `out/DimensionMasterColorIndependence20260921/README.md`,
`tested-inputs.json`, `gpu-color-comparison.csv`, `GradingWarp.cpp`에 있다. 실제 Client/UI,
전체 맵 pixel, Bloom blur, 화면의 시각 판정은 검사하지 않았다. 현재 실행 제품은1차 보정이며
DEC7 후보는 아직 정상 Product 빌드 전이다. 이 조사로 Q JSON, shader, 전역 profile/LUT 또는
Resources를 추가 수정하지 않았다.

## G08. 현재 렌더링을 기준으로 Q 금색과 윤곽 동시 보정

사용자는 렌더링 자체는 맞고 ALT V와 A 검격은 정상이라고 명확히 했다. 맵 grading bypass는
구현하지 않는다. Q/R/V 각각의 재질과 실제 재생 입력만 조정한다.

G05 DEC7 후보를 실제 current CS/Bern/G3 등7개 profile/region의 tone·LUT에 넣어 검증했다.
금색은 회복되지만 밝은 면의 포화로 edge 대비가 줄어 face gain0.35를 선택했다. 실제 source의
faceEmissionWeight만 `.0875 + .0525*grazing²`로 바꿨다. 배경 transmission/chroma, alpha,
caustic/edge는 보존했다. 정면 자체 HDR은 `(2.45,1.71,.0375)`, 측면은 `(3.92,2.736,.06)`이다.
최종 source SHA256은 `ccb5406e7efdf6544d7744228fd95c1af2ce5f8e79e151a5eb9d0b6d137fc7c9`다.

현재 source 그대로 실행한 GoldWarp2580검사는 failure0이며 opt-off 오차0, SceneBloom 선형
운반 최대 상대오차2.20109e-7, 자체 발광의 scene 의존오차0이다. 이전 DEC7 CSV/receipt는
`out/DimensionMasterQGold20260921/gpu/variant-gold-dec7/`에 보존했다. 멀티맵 결과와 최종
Product 빌드·설치 상태는 후속 기록에서 구분한다. 이 단계는 Q JSON을 다시 변경하지 않았다.

현재 source를 그대로 사용한7개 맵 profile/region의 GPU 출력5292개도 확인했다. gain0.35 탐색
결과와 최종 RGB1764개를 대조한 최대 절대 오차는6e-8이며 raw/own Bloom441개 최대 상대
오차는1.175e-7, alpha 오차는0이다. 최종 receipt는
`out/DimensionMasterColorIndependence20260921/QMapAdaptation/final-CCB5406E/final-verification.receipt.json`이다.
CS 회색 배경0.5/1/4의 금색 R−B는.343/.242/.170, edge−center 밝기차는.090/.083/.051이다.
Bern/G3의 배경4 edge−center 차이는.030/.026으로 맵마다 다르다. Bloom blur와 실제 화면의
가림·겹침을 포함한 결과는 아니며 모든 맵에서 동일한 색·대비를 보장하지 않는다.

## G09. V 시작 세 유리의 투명도 반영

G06 계획의 정확한 세 stable element만 기존 `fresnel_pow`를0.200000003에서0.5로 바꿨다.
현재 저장본 hash를 재확인하고 Windows ReplaceFileW로 백업과 원자 교체를 수행했다.
설치 JSON은 후보와 byte 동일하며 SHA256은
`c45d71b521f31b1bd63c6dc49d07fac1f0ec1d2508c0c21b0e43bd5fa66e3652`다.
43개 요소와 bloom0, 색·emissive·크기·시간·recipe·distortion·나머지 서식은 보존했다.

현재 codec을 다시 컴파일한 Load→Drawable→Save(out)→Reload→Drawable→canonical 동일성은
통과했다. native66/실제 DDS GPU384표본과 현재 tone/LUT 합성3584표본을 비교했다. 실제
Playback521개 World와 설치 WModel의 앞면 중심157105표본에서 음수 tangentView.z는0이다.
source camera anchor 조건의 세 occurrence 평균 alpha는.682/.716/.779에서.472/.518/.635로
감소하고 grazing alpha1 비율은16.4%/20.2%/30.9% 그대로다. native RGB를 낮춘 것이 아니라
면의 최종 합성 기여를 줄인 PROJECT_TUNED다. 실제 Client 카메라·가림·화면 판정은 미확인이다.

설치 및 최종 확인은 `out/DimensionMasterQRV20260921/v-install-20260921T015825559010Z/receipt.json`,
`v-final-verification.json`에 있다. 수치 근거는 `out/DimensionMasterVGlass20260921/`의
`comparison-summary.json`, `facing/playback.json`, `facing/facing-summary.json`을 따른다.

## G10. T 소환체의 환경 조명 복구

사용자 요청으로 `Effect_DocumentRenderer_Rendering.cpp`의 정확한 T 소환체 predicate가
선택하는 animated pass를11에서0으로 바꿨다. `2050500.unified / dimension_summon /`
`DimensionMaster_DimensionSummon.wmodel / sk_swp_dms_00_sk_sk_dimensionprison / MASKED_SURFACE`
조합만 해당한다. 기존 NONBLEND 제출과 mask0.3, VS/RS/DSS/BS는 보존한다. 일반 GBuffer의
diffuse/normal/specular를 복구하므로 맵 조명을 다시 받으며 tone/LUT도 기존대로 받는다.

기존 pass11은 diffuse를 emissive로 옮기는 unlit 경로였고 pass0은 정상 PS_MAIN이다.
공유 pass를 삭제하거나 번호를 바꾸지 않았고 다른 cue/ALT V/A/전역 renderer를 수정하지 않았다.
T unified의23개 요소와 소환체를 full restore했다고 기록하지 않는다. 원본 Action은2개 stage이며
현재 binding의 stage0과 stage1을 무조건 합치지 않고 원본 occurrence 대조를 계속해야 한다.

실제 소스·cue·catalog를 대조한16개 정적 조건은 통과했다. receipt는
`out/DimensionMasterTCurrent20260921/t-lit-contract.json`이며 GPU 화면 성공을 뜻하지 않는다.
원본 stage0의13개 PlayParticle occurrence는12개 system을 참조하며 현재23개 요소에는
4개 system만 남아 있다. 과거 삭제38개 visible 요소는 구 재질·socket·모델 재질 애니메이션
의미를 확인하지 않고 그대로 되돌리지 않는다. 원본과 현재·삭제 이력 및 미해독 경계는
`out/DimensionMasterTCurrent20260921/README.md`, `inventory.json`에 기록했다.

## G11. 우선 검증 빌드와 남은 경계

사용자가 반영 빌드를 먼저 검증하고 싶다고 요청하여 Q 최종 보정, V 시작 세 유리, T 환경
조명 복구만 우선 Product Debug 빌드를 시작했다. 정상 ALT V/A 문서와 Engine Renderer,
Deferred shader, RenderingProfiles는 이번 후속 시작 때의 hash와 동일하다. R tuning/full도
같고 기존 다른 세션의 dirty는 보존했다. 범위 대조는 `out/DimensionMasterQRV20260921/scope-verification.json`이다.

R은 실제2050180→Foldcut→tuning.restore의13개 요소가 current Codec/Playback에서 모두
생성된다. 앞쪽의 길쭉한 `purple-group-*-slash`3개는 자체 RGB(.48,.12,.90)와 낮은 rim alpha를
쓰며, 별도 `group-*-swing`3개는 SceneColor 없이 native260의 자체 HDR을 출력한다.
실제 DDS·현재 shader·Playback 입력2048개 GPU 표본에서 발광 출력을 확인했으나 mesh의
raster/cull/depth·실제 화면 가시성은 미확인이다. 원인 확정 없이 공통 재질이나 R 입력을
증폭하지 않았다. 조사와 구분 기준은 `out/DimensionMasterQRV20260921/r-review/README.md`에 있다.

변경된 Q 두 JSON과 V JSON parse 및 `git diff --check`는 통과했다. 빌드 로그는
`out/DimensionMasterQRV20260921/product-build/`에 기록한다. 실행 화면은 사용자 확인 경계이며
Client/UI를 자동 실행하지 않았다.

Product runner는 exit0/PASS이며 총370740ms, Client339718ms에 완료됐다. Engine/Shared/
Server/Client 컴파일과 정상 배포를 통과했다. Client는117개 OBJ와4개 CSO를 갱신했고
`Client/Bin/Debug/Client.exe`는11:04:54 KST, Server.exe는10:59:14 KST,
Q가 포함된 `Shader_VtxEffectMeshPreview.cso`는11:02:56 KST에 갱신됐다. 기존 FXC X4000/
X4008 등 shader 경고, C4819 인코딩 경고와 DirectXTK PDB LNK4099 경고는 있으며 오류는 없다.
범위 밖 경고를 이유로 공유 소스를 수정하지 않았다. Clean/Rebuild나 수동 CSO 복사는 하지 않았다.

정본 compile receipt는 `out/BuildPipeline/runs/20260921T020456173Z-debug-product.json`이며
runtime 필수 파일 누락·검사 실패는0이다. 이번 runner는 데이터 publish나 Client 실행을 하지
않았다. 설치 확인 `out/DimensionMasterQRV20260921/product-install-verification.json`에서
최종 Q/V/T 소스·데이터 hash와 갱신 EXE/CSO, 배포 Engine.dll의 원본 일치를 확인했다.
Q CSO SHA256은 `b2fbeafbc347a4eb29a95d56fe043f155624c1fbdb4f463aadeb62c7c285dcb3`이다.

검증 대상은 Q의 밝은 황금색 면과 테두리, V 시작 큰 유리의 투과·가장자리, T 소환체의 맵 조명
반응이다. R 수정과 T 전체 원본 복원은 이번 빌드에 포함하지 않았다. 실제 화면의 사용자 판정은
남아 있으며 정상 ALT V/A의 데이터를 바꾸지 않았다.

## G12. 2026-10-12 Q·R·A 큐브의 Bloom 누락 재분석

이번 대상은 실제 Q의 금색 큐브, R의 보라 큐브와 같은 재질을 사용하는 A의 보라 큐브다.
사용자는 프로젝트 종료 시점의 코드로 반복 수정 원인을 다시 분석하고 수정하도록 요청했다.
기존 G01~G11은 당시 증거로 보존한다. 현재 작업은 `codex/warlord-altv-radial-render-fix`에서
진행하며 시작 HEAD는 origin/main과 같았다. 다른 세션의 큰 dirty 변경은 보존하고 자동
stage/commit/push하지 않았다. 만료된 팀 LAN sync와 Client/UI 실행은 수행하지 않았다.

### 실제 소비 경로와 확인된 원인

`DimensionMaster.skillbindings.json`의2050100/Q,2050180/R,2050210/A는
`DimensionMaster.animevents`의 nailstrike/foldcut/willowrend에서 각각 아래 문서를 소비한다.

| 실제 문서 | CubeSample 요소 | 현재 저작 입력 |
|---|---|---|
| `effect.dimensionmaster.skill.2050100.tuning.restore` | `authored.source-particle.b4984eeabefb0fc93783ecda` | clarity1, emissive1, root Bloom 생략 기본1.3 |
| `effect.dimensionmaster.skill.2050180.tuning.restore` | `authored.tuning.dimensionmaster.r.purple-glass-sequence` | clarity 생략0, MIC `(2,.65,4,1)`, emissive1 |
| `effect.dimensionmaster.skill.2050210.a1.full.restore` | `authored.composition.dimensionmaster.a.hit-1-purple-cube`부터 hit-4까지 | 같은 보라 CubeSample, clarity 생략0 |

Q 비교용 `2050100.restore`의 Bloom16은 실제 Q tuning 문서의 값이 아니다. 현재
`Effect_DocumentRenderer_MaterialBinding.cpp`는 `g_EmissiveIntensity`를 바인딩하며
`Shader_EffectCubeSampleScene.hlsli`도 이를 곱한다. Q clarity의 독립 면 발광까지
현재 남아 있으므로 'Emissive 변수가 누락돼 Q 전체가0'이라는 원인 설명은 틀리다.

원본 CubeSample의 배경 입력은 현재 화면 HDR의 굴절 샘플이다. IBL 환경 cube를 선택하는
표준 Schlick Fresnel 구현과 혼동하지 않는다. 시선각 `facing=abs(normalize(V).z)`에 따라
`lerp(5*C^5,C,facing)`를 계산하고 MIC tint와 emissive를 곱한다. particle색의 원본
기여는 주로 edge이며 Q의 추가 faceEmission은 후속 프로젝트 보정이다.

Git `3fc23750` 이전에는 Deferred가 합성된 HDR에 `Extract_Bloom`을 적용했다. 이후
스킬별 Bloom 분리는 SceneColor reader에 `F(B)-F(black)+Write_SceneBloom(F(black))`을
적용했다. 이 식은 재질이 배경을 증폭하여 새로 threshold를 넘긴 기여를 잃는다.
또한 비선형 C^5에 이미 다른 문서의 배율을 적용한 B를 대입하므로 배경 Bloom 전달이
그 배율에 선형이지 않다. 이는 원본 RT0 수식 자체나 Fresnel 계수를 다시 추출해도
고쳐지지 않는 프로젝트 합성 경계의 결함이다.

09-15의 source tone/LUT 복원은 별도 변화다. 밝은 색 차이를 tone shoulder로 압축하고
일부 맵 LUT는 보라색을 회전시킨다. 09-21은 Q 본체 제한과 독립 면 발광으로 외관을
조정했지만 위 분리식을 남겼으며 R/A에는 Q clarity를 적용하지 않았다. 당시 최종
사용자 화면 PASS도 문서에 없다. 과거 개인 화면을 이번에 다시 관찰했다는 뜻은 아니다.

### 실제 수정

제품 수정은 `Shader_EffectCubeSampleScene.hlsli`와
`Shader_EffectMeshFamilyCarrier.hlsli` 두 파일이다. RT0 색·alpha·clip·메시·배치·재질
입력은 그대로 두고 CubeSample의 RT2만 현재 비음수 HDR 배경 C에서 계산한다.

```text
T = 현재 C와 기존 clarity에서 얻은 RGB 전달 계수
B = 이미 소유자별 가중치가 반영된 배경 Bloom
I = 현재 효과 문서의 Bloom 배율
X = 기존 배율 없는 Extract_SceneBloom
Bloom = clamp(T*B + I*max(X(F(C))-T*X(C),0), 0,60000)
```

첫 항은 기존 빛 번짐의 전달, 두 번째 항은 현재 재질이 새로 만든 bright-pass다.
항등 전달은 억제된 배경 Bloom을 재생성하지 않고 검은 배경에서는 자체 발광 추출을
유지한다. 이번에 추가한 invocation-local `g_CubeSampleBloomWritten` 표식으로 실제
CubeSample 분기가 RT2를 완성했음을 carrier에 전달한다. profile 번호만
검사하여 상위 StandardColor/RuntimeV2/reconstructed 분기를 가로채지 않는다.
다른 family의 출력 정책과 CBuffer·pass·프로젝트 등록은 변경하지 않았다.

이 식은 프로젝트의 개별 Bloom 소유권 정책 보완이며 원본 UE3 보조 MRT 복원이라고
표현하지 않는다. Q의 금색과 R/A 보라색을 모든 맵에서 똑같이 보장하는 색 보정도 아니다.
JSON·Resources·전역 rendering profile은 이번에 변경하지 않았다.

### 수치 검증과 화면 경계

`out/CubeBloom20261012`의 임시 창 없는 D3D11 WARP 비교는 실제 production include의
수정 전/후와 Engine의 기존 Bloom 함수, 설치된 caustic BC1 sRGB DDS를 사용했다.
648개 재질·clarity·배경·시선·UV·문서 배율 조합과6개 emissive 표본,32개 고HDR 경계에서
6,084개 조건이 통과했다. RT0 RGB·alpha의 최대 상대 차이는0, 고정 기능 alpha 합성 오차도0이며,
intensity0에서 가중 배경 Bloom 전달 선형성의 최대 상대 오차는1.472e-7이다.

배경 RGB0.4, 배경 Bloom0, 정면 중앙, threshold0.7/knee1, 문서 배율1.3의 GPU 표본:

| 입력 | 변경 전후 동일한 RT0 RGB | 이전 Bloom RGB | 수정 Bloom RGB |
|---|---|---|---|
| R/A 보라 emissive1 | `(.8,.26,1.6)` | `(0,0,0)` | `(.268124,.087140,.536249)` |
| R/A 보라 emissive2 | `(1.6,.52,3.2)` | `(0,0,0)` | `(.988002,.321101,1.976003)` |
| Q clarity1 emissive1 | `(3.792191,2.802927,.181306)` | `(2.275,1.587857,.034821)` | `(3.485490,2.536075,.134938)` |

따라서 보라 면의 발광 조절 무반응을 변경 전에 재현했고 수정 뒤 양의 응답을 확인했다.
Q에는 이미 자체 발광이 있었으며 누락된 추가 기여를 보완한 것으로 구분한다.
항등 재질의 배경 Bloom0 유지, 문서 배율0, clarity0/.5/1, alpha0/.6/1과 기존 blend
계산도 확인했다. 요약은 `summary.json`, 원시 결과는 `readback.csv`와
`minimal-emissive-response.csv`다. 이 검사는 실제 mesh의 raster/depth/시간별 입자·최종
tone/LUT·blur·게임 화면을 포함하지 않는다.

추가한 배경 HDR16/60000, clarity0/1, 배경 Bloom0/1, emissive1/0 경계는
`high-hdr-readback.csv`에 있다. RT0·alpha는 변경 전후 같고 RT2는 유한한0~60000으로
제한되며 실제 상한60000에 도달했다. emissive0은 색과 Bloom 모두0이다. FP32 시험
타깃에서 원래 RT0의 큰 값을 보존한 검사이며 제품 FP16의 원래 RT0 포화 개선을 뜻하지 않는다.

현재 `LevelRegistry.cpp`가 선택하는 Character Select warm-high-key, Bern neutral-day,
Kouku g1.base의 저작 및 게시 profile은 모두 `bloomEnabled=false`다. 실행 중 사용자
임시 설정은 이번에 관찰하지 않았다. Renderer의 Bloom pass와 최종 합성은 이 gate를
따르므로 OFF에서는 수정된 RT2도 화면 halo로 합쳐지지 않는다. 전역 설정을 임의로
켜지 않았으며, 사용자 비교는 Bloom ON/OFF 조건을 구분해야 한다. source tone/LUT로
인한 면의 색·대비와 Bloom 누락 해결은 서로 다른 판정이다.

변경 파일의 `git diff --check`는 통과했다. 전체 worktree 검사는 사용자/다른 세션의
기존 C++ 주석·선언12줄의 trailing whitespace로 실패했으며 해당 파일은 수정하지 않았다.
이번 변경에 JSON/XML은 없다. 제품 빌드와 실제 소비 CSO 검증은 아래 후속 항목에서
완료된 결과만 기록한다. Client/UI 실행과 최종 사용자 화면 판정은 미수행이다.

화면 비교는 Debug Client의 F1 → Rendering Workbench → Saved Settings →
Live rendering comparison의 `Bloom`을 세션 한정으로 켜고 실제 차원술사의 Q/R/A를
사용한다. 이 비교 체크는 Save Authored/Publish를 요구하지 않으며 `Reset comparison`으로
복귀할 수 있다. Effect Tool의 비교용 Q 대신 실제 `Q_튜닝`을 대조하고, R 보라 큐브와
A의4개 보라 큐브의 면/모서리·fade·겹침을 확인한다. 전역 옵션 저장 변경과 실제 플레이는
사용자 영역이며 에이전트는 실행하지 않았다.

## G13. 실제 제품 CSO의 변경 전후 출력

독립 검토는 include 단독 검사와 별도로 실제
`Client/Bin/Debug/Shader_VtxEffectMeshPreview.cso`를 FX11로 열어 VS·입력 레이아웃과
`AlphaTwoSidedDepthRead` pass,3개 MRT를 통해 창 없이 그렸다. 면 중앙의 평면 triangle을
사용하므로 이 검사는 설치 cube mesh의 방향·가림·전체 재생을 대신하지 않는다.

변경 전 설치 CSO(SHA256 `86480cd2e3f9248b5fbd037f9b61849d0b326cc0e96215de1777cd37e9625257`)는
24개 조건 중 보라 emissive1/2와 alpha.7/1의4개 조건에서 예상한 누락을 재현했다.
HDR 출력이 `(.8,.26,1.6)` 또는 `(1.6,.52,3.2)`인데 RT2는0이었다. 이는 코드 식을
CPU에서 다시 쓴 결과가 아니라 실제 변경 전 제품 PS_MAIN의 출력이다.

정상 Product Build가2026-10-12 14:17:31 KST에 갱신한 generic CSO
(SHA256 `792438d43ef386d728166031841b15879e0063c24225589530fbe15a13818424`)는
같은24개 조건에서 실패0이었다. 변경 전후24개 RT0 RGB의 최대 절대 오차는0이다.
보라 emissive1/2의 RT2가 G12 표와 일치하고 alpha.7에서 한 번만 가중되며 alpha0과
intensity0에서는 해당 기여가0이다. Q는 기존 자체 발광과 추가 기여를 함께 출력한다.
따라서 실제 carrier가 CubeSample의 RT2를 보존하는 연결까지 확인했다.

증거는 `out/CubeBloom20261012/actual-fx11-baseline.csv`,
`actual-fx11-current.csv`와 별도 FX11 probe source다. 현재 함수 분리·backend 우선순위와
검증 oracle도 독립 검토했으며 확인된 추가 결함은 없었다. 최종 Client 링크·배포 결과와
화면 승인은 별도이며, 이 시점은 generic shader 설치와 headless 출력 검증 완료다.

## G14. 2026-10-12 Product Debug 빌드 완료

`Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`는
exit0/PASS로 완료됐다. 전체779987ms이며 Engine795ms, Shared2041ms,
Server72648ms, Client701291ms다. Client는227개 OBJ,74개 CSO와 EXE를 갱신했다.
공통 carrier를 포함하는 shader의 정상 의존성 빌드이며 Clean/Rebuild, 수동 CSO 복사나
데이터 publish는 하지 않았다. 다른 기존 dirty 소스도 현재 작업 트리 그대로 빌드했다.

정본 receipt는 `out/BuildPipeline/runs/20261012T052159699Z-debug-product.json`이다.
필수 runtime 입력 누락0, 유효성 실패0이며42개 runtime data check가 통과했다.
확인 범위는 runner가 제공하는 파일 존재, Navigation 참조 및 Item/Valtan reward catalog
검사다. 모든 runtime domain의 플레이 검증을 뜻하지 않는다. 기존 FXC 경고, C4819/C4828
인코딩 경고와 DirectXTK PDB LNK4099 경고는 있으며 이 작업에서 광역 수정하지 않았다.

G13의 새 제품 CSO는 이 빌드가 설치한 산출물이다. Debug 컴파일·링크·배포와 창 없는
출력 검증은 완료했으며 Release 빌드, Client/Server 실행, 사용자의 Q/R/A 최종 화면
판정은 수행하지 않았다. 따라서 Bloom 출력 누락 수정 완료와 최종 외관 승인 대기를 구분한다.

## G15. 2026-10-12 Debug·Release 전체 반영 빌드 후속 검증

사용자가 두 구성의 빌드·검증을 모두 요청했다. 현재 작업 트리의 다른 변경을 보존하고
Product Build와 구성별 설치 파일·실제 CSO 검사를 수행한다. Client/Server/UI 실행과
최종 화면은 사용자 영역이며 rendering profile 저장값은 변경하지 않는다.

### Debug 완료

`out/BuildPipeline/runs/20261012T062459526Z-debug-product.json`은 exit0/PASS다.
총2484554ms, Engine446337ms, Shared322ms, Server61956ms, Client1972990ms이며
Engine CSO29개와 Client OBJ260개·CSO94개·EXE를 갱신했다. 기존 다른 변경의 Deferred와
모델 재질 의존성도 정상 Build에 포함됐다. 필수 runtime 입력 누락0·유효성 실패0,
runtime data check42개 PASS다. Clean/Rebuild나 수동 산출물 복사는 하지 않았다.

설치된 generic CSO를 다시 사용한 FX11 검사24조건은 실패0이며 이전 baseline 대비
RT0 차이0이다. CubeSample 소스 SHA256은 G12 검증본과 같은
`47149e61eb31abd7defb6d8ad7e8ddcdc3dc7a03298408bd4aea21eaca2cd183`이고,
CSO SHA256은 G13과 같은 `792438d43ef386d728166031841b15879e0063c24225589530fbe15a13818424`다.
증거는 `out/CubeBloom20261012/debug-release/Debug/receipt.json`과 `actual-fx11.csv`다.

전체 Product shader closure는 active producer260개, Client consumer171개,
family program148개, resource-root8조건과 V1/V2 각각1352픽셀 WARP 검사를 통과했다.
정본 `Test-CompiledShaderClosure.ps1`의 VS 탐색이 Preview를 제외하므로 out 전용 wrapper가
메모리 실행본의 `vswhere -prerelease`와 임시 출력 경로만 각각1회 보완했다. 원본 검사와
assert 본문은 유지하고 역치환 일치·원본 SHA256 전후 일치를 확인했다. 따라서 순정 스크립트
그대로의 실행 PASS가 아니라 같은 검증 로직의 실행 환경 보완 결과다. 저장소 도구 원본과
제품 설치 파일에는 쓰지 않았다. 증거는 `out/CubeBloom20261012/InstallClosure/Debug/`다.

첫 배포 fingerprint 검사는 다른 작업의 후속 Debug 빌드가 Engine DLL/PDB를 갱신하고
Client 배포를 끝내기 전이어서 두 쌍의 불일치를 기록했다. 외부 runner 부모 프로세스는
15:20:43 KST에 시작했지만 Engine MSBuild는 우리 Product 종료15:24:59 뒤인15:25:00에
시작했다. 실제 컴파일이 동시에 실행됐다고 해석하지 않는다. 해당 외부 빌드도
`out/BuildPipeline/runs/20261012T062923993Z-debug-product.json`의 PASS로 끝났다.

그 배포 후15:31:08 KST 재검사는 필수 산출물30개·배포 쌍14개 모두 PASS다.
`deployment-fingerprint-post-external-product-pass.json`과
`post-external-product-continuity.json`에 별도 보존했고 중간 불일치 증거도 삭제하지 않았다.
외부 Product의 네 단계는 모두 shaderWrites0이며 기존에 기록한 CSO13개와 실제 cube CSO의
hash도 동일하다. 전체260개 CSO의 전후 hash 목록을 새로 비교했다고 주장하지 않는다.

### Release 완료와 두 구성의 최종 대조

`out/BuildPipeline/runs/20261012T072341452Z-release-product.json`은 exit0/PASS다.
총3103589ms, Engine539256ms, Shared2346ms, Server67942ms, Client2491008ms이며
Engine OBJ75개·PCH1개·CSO29개, Shared OBJ9개·PCH1개, Server OBJ100개,
Client OBJ360개·CSO168개를 갱신했다. Client의 binaryWrites2는 EXE와 배포 DLL의
출력 변경 계수이며 실행 성공 횟수가 아니다. 필수 runtime 입력 누락0·유효성 실패0,
runtime data check42개 PASS다. 기존 경고는 남아 있지만 빌드 오류는 없다.

Release generic CSO SHA256은
`c8bf28d829cbc7bd95bc633a13b8ee9f07aaa345392a39dfc63b43854374a678`이다.
15:49:04 KST에 정상 FXC가 생성한 뒤 기존 probe EXE로 사전 검사했고, Product 배포 완료 후
hash 불변을 확인하고 Debug·Release 각각24조건을 다시 실행했다. 모두 exit0·실패0이며
두 구성의 RT0와 RT2 최대 차이는0이다. 각 `debug-release/<Configuration>/`의
`actual-fx11-final.csv`는 기존 CSV와 byte 단위로도 같다. `receipt.json`에 최종 Product
완료 근거와 배포 후 검증을 기록했다. Debug CSO도 기존 hash를 유지한다.

Release shader closure도 Debug와 같은 producer260개·Client consumer171개·
family program148개·resource-root8조건·V1/V2 각각1352픽셀을 통과했다. 실행 환경
보완의 범위와 원본 hash 불변 검사는 위 Debug와 동일하다. Release 검사는16:24:38 KST에
완료됐으며 `out/CubeBloom20261012/InstallClosure/Release/receipt.json`에 기록했다.

이후 read-only 최종 배포 검사는 Release29개 산출물·13쌍, Debug30개 산출물·14쌍 모두
PASS다. 각 `InstallClosure/<Configuration>/deployment-fingerprint-final-debug-release.json`에
보존했다. 검사 도구는 제품 파일을 다시 빌드하거나 복사하지 않았다. 큐브 소스 두 파일의
hash도 이 후속 작업 시작 때와 동일하며 전역 rendering profile과 Resources를 변경하지 않았다.

이번 기능의 변경 파일 `git diff --check`는 PASS다. 전체 작업 트리에서는 다른 기존 C++
주석·선언11줄의 trailing whitespace가 남아 있으며 이를 이유로 해당 파일을 정리하지 않았다.
생성한 검사 receipt는 JSON parse를 확인했고 제품 JSON/XML 변경은 없다. 임시 probe와
출력은 out 아래에 두며 빌드 산출물·검사 로그를 소스 commit에 넣지 않았다.

### 사용자 화면 확인 경계

Debug·Release 제품 컴파일·링크·배포와 설치·수치 검증은 완료됐다. Client/Server/UI는
실행하지 않았으며 원본 cube mesh의 실제 가림·재생과 최종 tone/LUT 이후 Q/R/A 화면은
사용자 확인이 남아 있다. 전역 Bloom OFF는 유지했다. 빛 번짐 비교는 G12의 Debug
세션 한정 Bloom 비교를 사용하며, Release에 같은 authoring 창이 있다고 안내하지 않는다.

확인할 실행 파일은 `Client/Bin/Debug/Client.exe`, `Client/Bin/Release/Client.exe`이며
직접 실행의 작업 디렉터리는 `Client/Default`다. Server도 각각
`Server/Bin/Debug/Server.exe`, `Server/Bin/Release/Server.exe`로 빌드돼 있다.
F5/Ctrl+F5는 VS 설정에 따라 Build를 수행할 수 있으므로 무빌드 실행이라고 표현하지 않는다.

## G16. 2026-10-12 초기 큐브 색감과 현재 후처리의 대조

사용자는 Bloom 누락 수정과 별도로, 전역 렌더링 설정을 유지하면서 Q/R/A 큐브만 최초 복구
당시의 색감에 가깝게 만드는 후속 작업을 요청했다. 명시 요청에 따라 Astra가 초기 기록과
합성 경계를 독립 조사했다. 이 항목은 조사와 후보 검증 기록이며, G15의 제품 빌드 성공을
아직 반영하지 않은 색 보정의 빌드 성공으로 확대하지 않는다.

### 최초 비교 기준

09-08 `CHARACTER_MATERIAL_AND_EFFECT_ROUND2_ESTIMATE_RESULT`의 첫 사용자 확인과
`사용자 화면 피드백과 Q/W 손튜닝 방향 검토`에 성공 Q2, 큰 노란 직사각형 유리와 발광 띠가
기록돼 있다. 당시 이미지 short ID는 `196df995`다. 이 PC에서 이미지 실물과 당시
`out/DimensionMasterQRestore20260908`는 찾지 못했으므로 이번에 당시 이미지를 직접
열람했다고 기록하지 않는다. 사용자에게 남아 있는 캡처를 요청했지만 조사의 선행 조건은 아니다.

가장 이른 Git 보관본은 `c1e7b50c91f4ac42c7b2b39e023649d650cb1cfe`의 09-09 02:36 KST다.
해당 CubeSample RT0는 현재 R/A 기본식과 같고 Q의 후속 clarity는 없다. 당시 Character Select
warm-high-key는 Hable, whitePoint1, gamma1.90499997, 명목 실효 exposure 약0.6351이었다.
Bloom은 ON, threshold2.74000001/knee1, 명목 실효 intensity 약2.2185이며 LUT 경로가 없었다.
이는 당시 저장된 설정이며 그 화면의 미저장 세션값까지 증명하지 않는다. 현재 보존된
before-restoration profile의 Bloom1.3을 당시 Git의2.61과 같다고 취급하지 않는다.

A의 보라 큐브는 `c67a47b2`의 09-10 보관본과 ROUND2 RESULT의 R donor 복사 기록을 따른다.
이는 원본 A emitter를 그대로 복원한 것이 아니라 R 재질을 사용하는 사용자 저작 조합이다.
이번 대상은 실제 Q tuning의 cube1개, R tuning의 purple-glass-sequence1개, A a1.full의
hit-1~4-purple-cube4개다. 같은 element ID가 있는 비교용 Q 전체를 자동 변경하지 않는다.

### 현재 후처리와 후보의 경계

`Shader_Deferred.hlsl::Resolve_FinalLDR`는 합성 HDR와 선택된 blurred Bloom을 더한 뒤
exposure, source tone, baked grading LUT를 적용한다. Character Select는 빈 원본 LUT라도
source tone을 받으며, 일부 쿠크 프로필은 실제 LUT로 보라색 방향을 바꾼다. Renderer의
BGRA8 grading atlas와 source tone의 saturate는 비가역일 수 있다. 따라서 단순 재질 RGB에
역 LUT를 적용하는 것을 모든 맵에서 정확한 초기 색을 복원하는 해법이라고 설명하지 않는다.

임시 `out/CubeColor20261012/analyze.cjs`는 실제 게시 프로필6개와 LUT DDS, 앞선 production
CubeSample의 GPU readback108행을 사용하여 CPU 근사3,888조건을 비교했다. source tone
역계산의 왕복 최대 오차는1.20e-11, 중성 LUT 격자 오차는0이다. 현재 검사 자체는 GPU final
shader나 실제 게임 화면 검증이 아니다. 초기 Bloom을 다시 켜지 않고 색 변환 차이를 분리한다.

추가 후보는 현재 배경의 표시색에 초기 Hable에서의 큐브 기여만 더하는 local-delta target과
비음수 재질 출력 제약을 포함한 역보정이다. 굴절 샘플 C와 실제 픽셀 배경 D를 구분한다.
한 occurrence의 모든 입자가 동일 snapshot을 공유하므로 같은 occurrence 안에서 큐브가
겹치면 D와 실제 blend 목적지가 달라진다. 단일 표본 오차 감소만으로 적용을 승인하지 않고
겹침, alpha fade, 색 방향과 대비를 별도 대조한다. 이 조사 뒤의 제품 연결은 G17에 기록한다.

## G17. 2026-10-12 Q/R/A 국소 색 응답 반영

### 실제 원인과 이전 시도의 차이

이번 문제는 두 단계로 구분한다. G15는 HDR SceneColor의 재질 증폭이 Bloom RT2에 누락된
합성 결함을 수정했다. G17은 그 뒤에도 남는 표시색 문제다. emissive 바인딩 자체가 사라진
것은 아니다. CubeSample의 배경 의존 `C^5`, MIC 색, caustic/edge에 emissive를 곱한 뒤,
복원된 `Resolve_FinalLDR`의 exposure/source tone/saturate/graded LUT를 통과한다.
채널이 포화되거나 LUT에서 비슷한 값으로 압축되면 emissive 증가가 색 차이·발광감을
회복하지 못한다. 원본 환경 cubemap의 Schlick Fresnel 오류라고 단정하지 않는다.

초기 성공 보관본 `c1e7b50c`(09-09)은 native CubeSample과 Hable/gamma를 조합했고,
`3fc23750`(09-13)의 공용 Bloom 분리, `579d9b90`(09-15, rendering restore)의 source
tone/LUT 연결을 거쳤다. Q에는 `f291f886`(09-21)에서 clarity 보정도 들어갔다. 최초 문제가 관찰된 정확한
commit이나 당시 화면의 미저장 설정까지 확인한 것은 아니다. 이번 변경은 또 다른 emissive
배수나 전역 exposure/LUT OFF를 적용하지 않고, 현재 후처리를 읽어 큐브의 국소 기여를
초기 Hable 기준에 가깝게 만든다. 원본 수식과 오늘의 표시 변환을 분리한 것이 차이다.

### 코드 연결

`Renderer::Bind_ActiveSceneColorTransform`은 현재 commit된 quality와 이미 생성된
grading SRV를 읽기 전용으로 제공한다. `Render_Final`과 source curve packing helper를
공유한다. LUT의 생성·수명·캐시·전역 설정은 바꾸지 않는다. GameInstance는 forwarding만 한다.

Client `Bind_MaterialInputs`는 generic mesh의 매 draw에서 선택 flag를 재설정한다.
실제 Q/R/A effect ID, Q1/R1/A4의 element ID, CubeSample profile42와 one/two-sided
alpha depth-read blend로 범위를 제한한다. 같은 ID의 additive/multiply 편집은 제외한다.
다른 CubeSample, 비교용 Q 문서, 도화가 F나 다른 클래스에는 자동 전파하지 않는다.

새 `Shader_EffectCubeColorResponse.hlsli`의 핵심은 다음 식이다. D는 굴절되지 않은 현재
픽셀 배경, S0는 Q clarity 이전의 native CubeSample 출력, a0는 원래 alpha, a는 현재
alpha, H는 초기 Hable, G는 현재 source tone/LUT다.

```text
target = saturate(G(D) + H(lerp(D,S0,a0)) - H(D))
G(lerp(D,S',a)) ~= target, 0 <= S' <= 60000
```

RGB만의 최근접 해가 모서리 휘도를 뒤집는 반례가 있어 RGB와 휘도 오차를 함께 쓰는
bounded 8회 solver를 적용했다. 비대상·alpha0·source OFF는 기존 RGB를 유지하며,
비유한 해나 기존보다 가중 오차가 큰 후보도 사용하지 않는다. Final과 같은 노출 전후
HDR60000 범위를 적용하고, NaN/INF 검사는 FXC fast-math에도 남도록 지수 bit를 검사한다.
기존 RT2를 먼저 계산한
뒤 RT0 RGB만 바꾸므로 앞선 Bloom 수식, alpha/clip/굴절 UV/깊이 상태는 보존한다.

### 확인 범위

현재 반영 코드에 대한 GPU·설치 CSO·Debug/Release 증거는 이 절의 후속 검증 기록으로
구분한다. G15 빌드를 새 색 보정의 빌드 성공으로 대신하지 않는다. Client/Server/UI는
실행하지 않았으며 최종 시각 판정은 사용자가 직접 한다.

LUT가 표현하지 못하는 색, 같은 occurrence 내부의 고정 snapshot 겹침, FXAA와 공간 Bloom,
현재 조명·배경과 과거 장면의 차이는 남는다. 전역 Bloom OFF를 유지했으므로 초기 ON 때의
halo 전체를 재현했다고 하지 않는다. 초기 캡처 실물이 없어 이번 기준은 보존된 코드·설정·
사용자 피드백 기록이며, 과거 이미지와 픽셀 단위 비교를 수행한 결과가 아니다.

### 최종 제품 함수 GPU 검증

`out/CubeColor20261012/gpu/final-receipt.json`은 제품 include SHA256
`792a20762fd3bdad84ca66e1d5ff6aeb110ea0355b5b01262adf49aa33743ee2`의 실제 함수로 검사했다.
실제 Deferred 함수와 게시 DDS에서 BGRA8 grading atlas를 만들고 HDR32/16 alpha blend를
각각7,872조건, 합계15,744출력으로 대조했다. 각 설정에는 유색 배경0~60000와 native
RGB1e10~1e30의 고휘도1,920조건이 포함된다. finite·출력 범위·flag/source OFF·alpha0
무변경·GPU 내부의 기존 가중 오차 보호 계약 실패는0이다. 별도의 O3 검사는 native/배경
각 채널의 NaN/Inf, subnormal, 최대 유한값 등72개 bit-pattern 조건을 통과했다.

실제 alpha·단일 합성의 평균 최대 RGB 채널 오차는 아래와 같다. 0~1 표시색에서 위의
역사 기여 target과 비교한 수치이며 과거 스크린샷과의 오차가 아니다. 각 칸은36표본이다.

| 프로필 | Q 금색 전 → 후 | R/A 보라 전 → 후 |
|---|---:|---:|
| Character Select warm-high-key | .23364 → .00029 | .05932 → .00027 |
| Bern neutral-day | .21077 → .00016 | .05078 → .00017 |
| 쿠크 G1 | .22037 → .00680 | .06889 → .00639 |
| 쿠크 G3 | .27253 → .04165 | .10929 → .03804 |
| Bern region106 | .22969 → .00092 | .06038 → .00341 |
| Bern region107 | .22563 → .00224 | .06726 → .00417 |

G3 유색 표본의 평균 hue 오차는 Q34.76°→5.01°, 보라53.90°→4.20°다. 단, G3 Q의
scene.5/facing.5에서는 edge−face 휘도가 단일−.0008342, 2겹−.0001931로 조금 반전된다.
각 차이는1/255보다 작지만 완전한 명암 보존 성공으로 기록하지 않는다. 5겹에서는 유의한
target 대비 반전은 없었다. 겹침 뒤의 목표 오차를 모두 줄였다는 보장도 없다. GPU solver와
CPU double solver는 float·LUT 미분·분기 및 역 tone 구현 차이가 있으므로 CPU 후보 일치 자체를 통과 조건으로
삼지 않았다. 관련 상세는 `production-outcomes.json`, `production-receipt.json`에 있다.

AMD Radeon 840M에서 별도 창 없는 timestamp 검사는256×256의 단색 synthetic 입력으로
최종 solver만 측정했다. ON 중앙값 범위.03336~.12849ms/draw, 추가 비용.03001~.12528ms/draw,
108 timed batch의 disjoint0이다. scene copy·실제 mesh·carrier·가림·파형별 분기 비용을
포함한 게임 프레임 측정이 아니므로 FPS 개선이나 실제 최악 비용으로 환산하지 않는다.
FXC X4000의 첫 guard 반환 경고는 남고 실제 경계 검사는 통과했다. 기존 경고를 숨기기 위해
전역 compiler 옵션을 바꾸지 않았다.

최종 독립 검토에서는 새 구현 결함을 확인하지 못했다. 다만 ON/OFF timestamp 비교는 같은
새 generic shader 안에서 수행했다. 공유 PS의 bytecode/register footprint 증가가 비선택
generic mesh에 주는 이전 shader 대비 비용은 분리 측정하지 않았다. 선택 픽셀 solver의
위 수치를 모든 이펙트에 대한 성능 무영향 보장으로 사용하지 않는다.

### 선택 조건 검증

`out/CubeColor20261012/installed/selector-run-receipt.json`은 실제 Client의 selector 식과
실제 enum을 추출·컴파일한120조건을 실패0으로 통과했다. 6개 exact ID와 두 alpha profile,
잘못된 문서/element, profile0/41/43/127, opaque/additive/multiply/END를 포함한다.
원래 데이터의 animevents1543/1546/1547와 direct-authored document staging도 대조했다.
준비 문서의 ID 일치 검사 때문에 다른 V2 ID로 바뀌어 selector를 놓치는 경로는 확인되지 않았다.

### 설치된 Debug/Release FX11 검증

정식 빌드가 설치한 `Shader_VtxEffectMeshPreview.cso`를 D3D11 WARP의 실제 FX11
`AlphaOneSidedDepthRead` pass로 실행했다. Debug/Release 각각288 draw, 실패0이다.
warm/Bern/G3, 금색/보라, emissive1/2, Bloom0/1.3, alpha0/.7/1과 OFF/ON/OFF/source OFF를
대조했다. 선택된 RT0 RGB는 바뀌고 RT0 alpha, RT1 RGBA, RT2 RGBA는 구성별 memcmp로
bit-exact 보존됐다. OFF 복귀와 source OFF/null LUT도 이전 출력으로 bit-exact 복귀한다.

두 구성의288개 readback 행을 대조한 최대 절대 차이는 모든 출력에서0이다. 이 구성 간
비교는 float round-trip9자리 CSV 기준이며, 구성 내부 bit-exact 검사와 구분한다.
각 검사 전후 CSO hash도 동일했다.

| 구성 | 설치 CSO SHA256 |
|---|---|
| Debug | `C31E7044609EBA6ABEB2A54A1D9D3DB1745FE394CE8395C726595BE75B4DDD80` |
| Release | `04361461ADCF3B0A236EA6820E9A6535CA4CB3FB150495A05875F1EC5FC42C94` |

증거는 `out/CubeColor20261012/installed/receipt-debug.json`, `receipt-release.json`,
`cross-configuration.json`이다. 이 검사는 synthetic triangle과 RGBA32F MRT의 실제
carrier 소비자 검사다. final tone/LUT 및 HDR16 합성은 앞 절의 별도 GPU 검증에 포함한다.
게임의 실제 메시·가림·공간 Bloom·화면을 검증한 것으로 확대하지 않는다.

### 최종 Product 빌드와 보존 검사

최종 소스 기준 x64 Product Build는 Debug/Release 모두 PASS다. Clean/Rebuild나
수동 CSO 복사는 하지 않았다. 두 구성 모두 Engine/Shared/Server/Client 단계 PASS,
runtimeDataChecks42개 PASS, missingRuntimeInputs0, invalidRuntimeInputs0이다.

| 구성 | 최종 receipt | 시작 → 완료 KST |
|---|---|---|
| Debug | `out/BuildPipeline/runs/20261012T090723606Z-debug-product.json` | 17:59:04 → 18:07:23 |
| Release | `out/BuildPipeline/runs/20261012T092048478Z-release-product.json` | 18:09:06 → 18:20:48 |

Release는 이번 작업에서 `Invoke-BuildAndRegression.ps1 -Configuration Release -Profile
Product -MaxCompilerProcesses 4`로 실행했다. Debug의 최초 Product Build도 성공했지만,
최종 경계 수정 뒤 재실행은 다른 작업의 headless Server 점유와 공유 Debug build lock
대기로 각각 중단됐다. 해당 프로세스를 종료하거나 잠금을 우회하지 않았다. 대신 같은
workspace에서 최종 수정 뒤 실행된 CodeReadability 작업의 위 Debug Product receipt를
확인하고, 그 설치 CSO를 별도의288-draw 검사로 재검증했다. 잠금 timeout을 컴파일 성공으로
기록하지 않는다. Debug 로그는 `out/CodeReadability/session-20261012/product-debug-4.log`,
Release 로그는 `out/CubeColor20261012/release-final-build.log`다.

빌드 완료 뒤 제품 include와 두 설치 CSO의 hash는 위 GPU/설치 receipt와 일치했다.
PLAN의 G17 전문과 실제7개 소스 hash도 일치한다. `Client.vcxproj`와 `.filters` XML parse,
새 include의 단일 등록을 확인했다. 변경한 shader/Client/project/문서는 `git diff --check`
PASS다. Engine 파일의 기존 무관한 변경에는 `GameInstance.cpp:80`, `GameInstance.h:32`의
trailing whitespace2건이 남아 있어 전체 Engine diff를 무경고라고 하지 않는다. 이번 API
추가 행과 신규 include에는 trailing whitespace가 없다. 기존 C4819/C4828 등 인코딩 경고와
FXC X4000은 남지만 빌드 오류는 없다. 기존 C++ 인코딩을 일괄 변환하지 않았다.

조사 시작 receipt의 hash와 비교하여 저작/게시 RenderingProfiles, Deferred shader,
RenderingProfileService, 실제 G1/G3 LUT DDS가 모두 그대로임을 다시 확인했다. 이 작업은
effect JSON이나 Resources를 수정하거나 publish하지 않았다. 전역 Bloom/exposure/LUT/
FXAA/안개 및 팀장 튜닝값도 변경하지 않았다. 사용자와 다른 작업의 변경은 보존했다.

### 사용자 화면 확인 경계

구현·컴파일·설치 셰이더 소비자·수치 검증까지 완료했다. Client/Server/UI를 이 작업에서
실행하지 않았으므로 최종 화면 확인은 미완료다. 사용자는 현재 빌드에서 Q 금색 면/테두리,
R 보라 큐브, A의4회 보라 큐브를 같은 카메라와 배경에서 확인한다. 특히 어두운 배경과 밝은
배경, 큐브 겹침과 사라지는 구간의 색·명암을 함께 확인해야 한다. 기존 EXE가 실행 중이면
새 바이너리의 반영 여부와 화면 판정을 구분한다.

최초 캡처 실물은 찾지 못했으므로 코드·설정 기록 기반의 색 기여 복원이다. LUT의 표현 범위와
고정 snapshot 겹침 때문에 과거 화면과 완전 일치를 보장하지 않으며, 현재 Bloom OFF를
유지하는 조건에서 초기 Bloom ON의 공간적인 빛 번짐까지 복원했다고 설명하지 않는다.

## G18. 2026-10-13 Q 복구 확인과 Q/R/A 강한 발광 반영

### 사용자 판정과 실제 원인

사용자가 G17 이후 Q는 잘 나오는 것 같다고 복구를 확인했고 R/A 보라 큐브는 이전과 같다고
알렸다. 후속 지시는 emissive를 소폭이 아닌 크게 올리고, 실제 수정 데이터·수치·코드 위치와
Debug/Release 빌드까지 제공하는 것이다. G17 Q 확인과 이번 강화 후보의 화면 승인은 구분한다.

현재 `DimensionMaster.animevents`1543/1546/1547 → EffectCatalog의 DIRECT_AUTHORED_DOCUMENT
→ exact document/element → SourceProfile42 → GENERIC/MESH → CubeSample까지 재대조했다.
R/A의 selector 누락이나 새 material packet으로 조용히 우회하는 경로는 확인되지 않았다.
Product registry는 빈 registry이며 document ID는 요청 ID와 일치해야 준비된다. 설치된 두
CSO도 G17 기록의 hash와 같았다. 과거 out 검증 파일은 없어 이번 실행 증거로 재사용하지 않았다.

문제는 보정 기준의 한계였다. R/A는 `project_clarity_strength`가 생략되어0이고, 정면 중앙의
본체는 배경 의존값이다. 검은 배경에서는 caustic과 edge가 없는 중심의 RGB도0이므로
emissive4/8을 곱해도0이다. Q에는 별도 faceEmission이 있지만 G17은 clarity 이전의
nativeColor/sourceAlpha를 목표로 삼아 그 저작 발광을 다시 약화하거나 제거할 수 있었다.
한편 초기 Hable 기준은 HDR 약1.574555부터 각 채널이1로 포화해 밝은 면에서 gain만 늘려도
흰색 target이 그대로 남는다. 이는 emissive GPU 바인딩 실패와 다른 문제다.

### 실제 변경 데이터

수정한 정본은 다음 세 파일이다. 공통 수정 필드는 해당 stable element의
`detail.color.emissiveIntensity`이며 **1 → 4**다. 일반 particle color, lifetime, transform,
텍스처, 발생 수와 다른 element는 변경하지 않았다.

| 정본 파일: Data/Effects/Authored/ 아래 | 대상 element | 추가 변경 |
|---|---|---|
| `effect.dimensionmaster.skill.2050100.tuning.restore.effect.json` | `authored.source-particle.b4984eeabefb0fc93783ecda` | Q clarity1 유지 |
| `effect.dimensionmaster.skill.2050180.tuning.restore.effect.json` | `authored.tuning.dimensionmaster.r.purple-glass-sequence` | clarity 생략0 → 1 |
| `effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json` | `authored.composition.dimensionmaster.a.hit-1-purple-cube`부터 hit-4까지4개 | 각각 clarity 생략0 → 1 |

R/A의 추가 필드는 `material.sourceProfile.scalars`에 들어가는 기존 named scalar다.

```json
{ "name": "project_clarity_strength", "group": "Project tuning", "value": 1 }
```

강도 조절은 위 emissive 수치를 수정하면 된다. `detail.linearLerp.emissiveIntensity=false`이므로
같은 이름의 boolean이나 현재 미사용 `endEmissiveIntensity=1`은 증폭값이 아니다. 이펙트 문서의
Bloom intensity나 전역 exposure/Bloom을 대신 올리지 않았다. 비교용 Q restore는 별도 문서이며
이번에는 실제 스킬 cue가 소비하는 tuning 정본을 변경했다.

4배는 입력 HDR 에너지 배수이며 화면 밝기가4배라는 뜻이 아니다. 8배 후보는 CS 정면 중심의
회색 배경.5에서 보라 target이[1,1,1]로 포화해 제외했다. 같은 조건의4배도[1,.85191,1]로 밝은
연보라가 되므로 모든 배경의 진한 보라를 보장하지 않는다.

### 실제 수정 코드와 유지한 소비자

`Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli::Shade_EffectCubeSampleScene`은
clarity 이전의 `nativeColor` 임시변수를 없애고, RT2 계산 뒤 현재 저작 SceneColor RGB/alpha를
`CubeColor_Resolve`에 전달한다. clarity는 배경 transmission을 제한하고 faceEmission을 더한다.
원래 sourceAlpha clip은 유지한다.

`Client/Bin/ShaderFiles/Shader_EffectCubeColorResponse.hlsli::CubeColor_Resolve`는
nativeMaterial/nativeAlpha 인자를 제거하고 현재 재질과 실제 alpha로 아래 목표를 계산한다.
solver 반복·오차 보호·현재 LUT 입력·전역 후처리는 바꾸지 않는다.

```text
target = saturate(G(D) + H(lerp(D,Sauthored,a)) - H(D))
```

G17의 원본 기준 보정에서 현재 저작 결과를 존중하는 PROJECT_TUNED 색 응답으로 바뀐다.
Q의 승인된 금색 면도 더 밝게 만드는 의도적인 변화이며 완전한 외형 불변으로 설명하지 않는다.
R/A clarity는 독립 면 발광뿐 아니라 밝은 배경에서 중간 alpha를최대0.7→0.91로 강화한다.
fade0/1과 원본 clip은 유지한다. 따라서 어제 재질 대비 alpha/RT2가 불변인 수정은 아니다.
색 solver 직전·직후의 alpha/RT2만 동일하다.

검토했지만 수정하지 않은 소비자는 다음과 같다.

- `Effect_DocumentCodec_MaterialValidation.cpp`: 기존 named scalar finite/unique 및 clarity0~1 검증.
- `Effect_DocumentRenderer_ResourceStaging.cpp`: profile42에서 named clarity를 `vSourceScalars0.w`에 저장.
- `Effect_DocumentRenderer_MaterialBinding.cpp::Bind_MaterialInputs`: exact Q/R/A selector,
  `g_SourceScalars0`, `g_EmissiveIntensity`와 Renderer의 현재 tone/LUT를 바인딩.

큐브 기능의 새 C++/schema/shader constant/project 등록은 없다. 두 shader와 데이터3개가
큐브 변경 범위이며, 각 코드 전문과 데이터 교체 블록은 같은 PLAN의 G18에 있다.
별도로 전체 빌드를 막은 `Client/Public/BossLogicFlowView.h:8`의 잘린
`#include <string_vie`를 `#include <string_view>`로 한 줄만 복구했다.
UTF-8 BOM과 다른 작업의 기존 변경은 보존했다. 이 수정은 큐브 로직과 무관한 문법 복구다.

### 저장·구조·GPU 검증

사용자의 반영 요청에 따라 현재 디스크 정본으로 후보를 만들고 직전 hash 재확인·백업·원자
교체했다. `out/CubeColor20261013/data-applied.json`과 `data-backup/`에 기록했다. 미저장
메모리 draft의 자동 병합·Reload는 하지 않았다. direct-authored 문서이므로 별도 publish나
DataFiles/Resources 복사는 필요하지 않으며 실제로 하지 않았다.

독립 검사 `candidate-independent-review.json`: 세 문서6개 요소에서 emissive6개와 clarity5개,
정확히11개 필드 변화만 허용했다. 그 밖의 구조는 동일하다. 기존 문서별 validator5종과
CubeSample 계약/finite·unique scalar/범위/selector 검사를 통과했고, 문서별 리소스8/18/45개는
모두 존재·nonempty이며 참조 목록이 그대로다. 적용 직후 hash도 검증 후보와 일치한다.

최종 제품 함수 자체를 컴파일한 별도 창 없는 D3D11 WARP 검사는 O3의2,160 draw를 실패0으로
통과했다. 현재6개 profile, Q/보라, 배경0/.1/1/4/16, 면/edge, facing1/.5, alpha실제/0/1,
ON/selector OFF/source OFF를 대조했다. shader hash는 CubeSample
`be4a7f82fe588669e6a6a5b50971883dd0c19756f9a019467d0a72b407b1e770`, ColorResponse
`9c8ee99e9497034b03dc4fbce8e01d435b9e060b0996244811245ebeb8a1047e`다.

solver 전후 RT2와 alpha2,160건 및 비활성/alpha0의1,680건은 bit-exact 동일했다. 활성480건 중
474건은 target 가중 오차가 감소했고6건은 동일하며 비유한값·범위·기존 오차 악화는0건이다.
CS의 보라 정면 중앙 alpha.7에서 실제 GPU 표시색은 다음과 같다.

| 배경 HDR | 최종 RGB |
|---|---|
| 0 | [.592708, .337224, .817995] |
| 1 | [.899203, .655974, 1] |
| 4 | [.952604, .688787, .973934] |

증거는 `out/CubeColor20261013/gpu/receipt.json`이다. 실제 제품 HLSLI 함수, 실제 DDS의
CPU grading bake와 GPU 합성식/tone을 검사했지만, synthetic constant caustic이며 설치 FX11
CSO·고정기능 blend·게임 장면의 검사와 같지 않다. HDR16에서는 target 자체가 흰색으로
포화하고 G3 보라의 평균 최대채널 오차.06845가 남는다. 면/edge120쌍의 edge 휘도 차이 최솟값은
-.0000881이며 완전한 대비 보존을 주장하지 않는다. 최종 화면과 실제 게임 성능은 미확인이다.

### 파일별 검토·수정 위치 요약

| 구분 | 실제 파일과 위치 | 책임과 이번 변경 |
|---|---|---|
| 수정 | `Data/Effects/Authored/effect.dimensionmaster.skill.2050100.tuning.restore.effect.json:25` | Q 실제 cue의 `detail.color.emissiveIntensity` 1→4 |
| 수정 | `Data/Effects/Authored/effect.dimensionmaster.skill.2050180.tuning.restore.effect.json:186` | R 보라 emissive 1→4, 같은 요소의 material scalar clarity1 추가 |
| 수정 | `Data/Effects/Authored/effect.dimensionmaster.skill.2050210.a1.full.restore.effect.json:17378,17591,17804,18017` | A hit-1~4 보라의 emissive를 각각1→4, 각 material scalar clarity1 추가 |
| 수정 | `Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli:130` | clarity·faceEmission·coverage 적용 결과를 solver로 전달 |
| 수정 | `Client/Bin/ShaderFiles/Shader_EffectCubeColorResponse.hlsli:94` | `CubeColor_Resolve` 목표를 현재 저작 RGB/alpha로 계산 |
| 검토·유지 | `Client/Bin/ShaderFiles/Shader_EffectCubeSampleScene.hlsli:100` | 면 발광의 실제 계산식과 emissive 곱셈. 기존 계수는 유지 |
| 검토·유지 | `Client/Private/Effect_DocumentCodec_MaterialValidation.cpp:791` | 기존 clarity0~1 검증 |
| 검토·유지 | `Client/Private/Effect_DocumentRenderer_ResourceStaging.cpp:535` | clarity를 `g_SourceScalars0.w` 입력으로 준비 |
| 검토·유지 | `Client/Private/Effect_DocumentRenderer_MaterialBinding.cpp:1103` | `Color.fEmissiveIntensity`를 `g_EmissiveIntensity`에 실제 바인딩 |
| 별도 문법 복구 | `Client/Public/BossLogicFlowView.h:8` | 잘린 표준 헤더 include 한 줄 복구. 큐브 동작 변경 아님 |

### 최종 Debug·Release Product Build

공식 명령 `Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Debug/Release -Profile
Product -MaxCompilerProcesses 4`를 각각 실행했다. Clean/Rebuild, 수동 CSO 복사와 데이터
publish는 하지 않았다. 최종 두 회차 모두 Engine/Shared/Server/Client PASS이며,
runtimeDataChecks42/42 PASS, missingRuntimeInputs0, invalidRuntimeInputs0이다.

| 구성 | 최종 receipt | 시작 → 완료 KST |
|---|---|---|
| Debug | `out/BuildPipeline/runs/20261013T101845630Z-debug-product.json` | 19:17:08 → 19:18:45 |
| Release | `out/BuildPipeline/runs/20261013T102104331Z-release-product.json` | 19:21:00 → 19:21:04 |

Debug 최초 회차 `20261013T101536055Z-debug-product.json`은 shader 컴파일 뒤 위 include 오류로
FAIL이었다. 한 줄 수정 후 최종 Debug에서 Client OBJ29개와 바이너리2개가 생성됐다.
Release는 동일 workspace의 앞선 실패 회차에서 최종 shader가 생성됐고, 수정 뒤 다른 작업의
`20261013T102027662Z-release-product.json` PASS가 Client 바이너리2개를 생성했다.
이 작업의 최종 Release 실행은 그 상태를 정상 증분 Build로 재확인해 PASS이며 output writes0이다.
실패 회차를 전체 빌드 성공으로 기록하거나 최종 Release에서 전부 재컴파일했다고 설명하지 않는다.

설치된 `Client/Bin/Debug/Shader_VtxEffectMeshPreview.cso`는19:15:06 KST 생성,
3,137,504 bytes, SHA256 `41d062e32e02e5bde74fabd71d4f7012b2044501b63fc839819e93e96a95e84d`다.
Release의 같은 CSO는19:16:39 생성, 601,032 bytes,
`29c3a3ceea2c9ca4310d67b054e9a05043dcdb13dd10a6448e15f8d6b443e3a9`다.
둘 다 최종 HLSLI 수정 이후 생성됐고 최종 PASS가 최신으로 인정한 구성별 산출물이다.
Debug/Release hash는 서로 다르며 정상적인 구성 차이를 동일 hash라고 쓰지 않는다.
이 CSO의 실제 Client draw까지 검사한 것은 아니다.

로그는 `out/CubeColor20261013/build-debug-final.log`, `build-release-final.log`다.
기존 C4828 인코딩, DirectXTK PDB LNK4099 및 shader X4000 경고는 남아 있다.
제품 함수 GPU receipt의 sourceHashes4개와 최종 파일이 일치하고, global-before의 저작/게시
RenderingProfiles, Deferred shader, RenderingProfileService, G1/G3 LUT6개 hash도 모두 같다.
전역 tone/LUT/Bloom/FXAA/안개 및 Resources는 이 작업에서 변경하지 않았다.
적용 데이터3개의 최종 hash·JSON parse·6개 대상 emissive4/clarity1, PLAN에 싣는 소스3개의
hash를 다시 확인했다. 변경 범위 `git diff --check`와 미추적 ColorResponse 파일의 별도
trailing whitespace 검사도 PASS다. 무관한 전체 작업 트리의 무경고를 주장하지 않는다.

### 남은 사용자 화면 확인

코드·데이터 반영, 구조·GPU 수치 검사와 두 구성 빌드는 완료했다. Client/Server/UI는 실행하지
않았다. 새 실행 파일에서 Q 금색과 R/A 보라의 면·테두리·fade를 밝고 어두운 배경에서 확인한다.
실행 중 도구의 미저장 draft를 자동 Reload하지 않았으므로 설치 파일 반영과 메모리 상태는 별도다.
전역 Bloom OFF는 유지했으므로 공간적인 빛 번짐을 켰다는 뜻이 아니다. 이번4배 저작값의
최종 색감·눈부심·겹침 판정은 사용자 확인을 기다린다.
