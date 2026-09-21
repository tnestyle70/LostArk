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
