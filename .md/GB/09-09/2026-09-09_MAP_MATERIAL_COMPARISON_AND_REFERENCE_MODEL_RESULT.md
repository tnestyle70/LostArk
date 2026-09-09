# 맵 A/B 비교·인형 모델 실행 체크포인트

작성일: 2026-09-09. 상태: **Debug Product 빌드·실제 CModel 검사 완료 / 사용자 화면 확인 전**.
사용자의 실행 준비 요청으로 변경 범위를 고정했다. 전체 맵·캐릭터 복원과 발탄 Composition 확대가 완료됐다는 뜻은 아니다.

## G00. 실제 반영

| 대상 | 반영 상태 |
|---|---|
| Rendering Benchmark | Capture A: legacy / Capture B: recovered, 같은 조건의 B−A 비교와 JSON 기록 |
| 인형 MN_CDMD_00 | 모델 1개, 원본 clip 30개, diffuse/normal/specular/emissive 4개 |
| Object Tool·Composition Resources | 기존 objectResources·Motion 소비자에 등록, WorldSequences 413 publish |
| 기존 이펙트·셰이더 분리·Play All | 공유 작업 트리 변경을 유지하여 Product 빌드에 포함 |

기존 C++ 변경은 `RenderingBenchmark.h/.cpp`, `MainApp.cpp`다. 새 C++/project/filter 항목은 없다.
대규모 공유 dirty tree에서 다른 변경을 되돌리거나 자동 stage/commit/push하지 않았다.

## G01. 비교 계약

A/B는 기존 `bUseSourceMaterials`를 변경하며 Begin 실패 시 이전 설정을 돌려놓는다.
level, viewport, view/projection, debug view, quality, fog, shadow와 scene light의 이름 있는 필드를 기록한다.
측정 중 조건이 바뀐 run은 제외하고 같은 조건·반대 mode만 비교한다. 구조체 padding은 사용하지 않는다.
GPU 표본이 부족하면 GPU 비교 불가를 표시한다. 기존 activation frame 제외·GPU 지연 처리를 유지했다.

이 스위치는 이미 source material에 연결된 표면에 작동한다. 전체 맵 복원을 추가한 기능은 아니다.
정적 비교는 사용자가 animation을 멈추고 카메라·설정을 유지한다. 실제 버튼 입력·결과 JSON 저장 화면은 사용자 확인 전이다.

## G02. 원본 모델과 로드 확인

원본 `mn_cdmd_00.Mesh.MN_CDMD_00_SK_LOC_INT`와 `MN_CDMD_00.Ani.MN_CDMD_00_Ani`를 추출했다.
UModel LZ4 오류는 out의 비압축 작업용 package로 해소했고 설치 원본은 변경하지 않았다.
원본 joint 57개, 네 foot chain과 PSA clip 30개를 확보했다. converter는 scene root를 포함해 bone 58개를 생성했다.
staging 단위 변환은 100배, 기존 runtime `modelPreScale=0.01`이다.

첫 cook는 실제 CModel에서 invalid bone data로 거부됐다. normalized UBYTE weight가 converter에서
원본과 다르게 변환된 것을 확인했다. 원본 정규화 값을 FLOAT accessor로 변환해 재cook했고
9,301개 정점의 위치 대응·bone 이름별 양수 influence/weight가 모두 일치했다.
최종 WModel의 수동 byte 보정이나 runtime 검증 완화는 없다.

| 검사 | 결과 |
|---|---|
| 실제 WARP CModel prototype → clone | PASS, mesh 1, bone 58 |
| animation | 30 clip × 5시점 = 150 pose, 모든 bone matrix 유한값 |
| 네 다리 | bip001/002 좌우 foot chain 4개 |
| 실제 Material texture | diffuse/normal/specular/emissive 4개 로드 |
| 기본 Motion | att_battle_2_01: 회전 화염 뿜기 **모델 동작** |
| 별도 화염 particle·socket 발생·Sound | 아직 연결하지 않음 |

Object ID는 `world.object.kouku.odd_doll`, 표시명은 `괴기스러운 인형 · 네 다리 화염 모델`이다.
기존 object/30개 Motion template·instance 소비자에 등록했다. 기존 모든 row는 의미상 그대로 보존했다.
Authoring 412→413과 runtime 413은 동일하고 WorldSequences scope만 Validate/Publish했다.
Object/Composition의 실제 선택·Append·Preview·Save/Reopen은 사용자 확인 전이다.

물리 위치는 `Client/Bin/Resources/Character/KoukuSaton/MN_CDMD_00/`다.
Resources-relative ID는 다음과 같다. binary는 Git에 추가하지 않았으며 **Drive 전달은 하지 않았다**.

- `Character/KoukuSaton/MN_CDMD_00/MN_CDMD_00.wmodel`
- `Character/KoukuSaton/MN_CDMD_00/textures/mn_cdmd_00_d_loc_int.dds`
- `Character/KoukuSaton/MN_CDMD_00/textures/mn_cdmd_00_n_loc_int.dds`
- `Character/KoukuSaton/MN_CDMD_00/textures/mn_cdmd_00_s_loc_int.dds`
- `Character/KoukuSaton/MN_CDMD_00/textures/mn_cdmd_00_e_loc_int.dds`

## G03. 남은 범위

| 범위 | 조사·미완료 상태 |
|---|---|
| 쿠크 전체 | 사용 asset 305·slot 379·원본 material 216개 조사. 계열별 shader/입력의 새 runtime 연결 미반영 |
| 발탄 아레나 | 후보 asset 138·slot 175. exact join material 135개, 오류 2건. 추가 재질 배포 없음 |
| 베른 camera 주변 | 후보 asset 286·slot 452. exact join material 269개, 오류 106건. 추가 UV/baked 입력과 미해결 대응 필요 |
| 세 캐릭터 환경반사 | 원본 hierarchy/program 조사. cube/상수/LUT·추가 UV 연결 필요. 차원술사 hair의 양수 가중치 bone 11개가 body에 없어 skeleton·animation set 확장 필요. 기존 1.5배 유지 |
| 발탄 Composition | Source-only Save·Product snapshot·V2 identity/clock·row/Draft/UI 연결 미완료. 기존 Save 유지 |

조사 자료는 `out/KoukuFullMaterialRestore20260909/`, `out/CameraMapMaterials20260909/`,
`out/CharacterMaterialParity20260909/`다. 카메라 후보 수는 배치 노출 변경이나 전체 exact coverage 완료가 아니다.
발탄 Python 초안 3개와 patch는 `out/ValtanCompositionParity20260909/unfinished_source_save/`에 보존했다.
해당 초안만 작업 전 상태로 돌렸고 다른 세션 변경은 유지했다.

## G04. 검증과 실행 인계

`Invoke-BuildAndRegression.ps1 -Configuration Debug -Profile Product`: **PASS, 13.110초**.
Engine/Shared/Server/Client compile/deploy 성공, missingRuntimeInputs 0개. 기존 C4819 경고는 남아 있다.
이번에는 shader 변경 없이 compiled shader를 재사용했다. 이 시간은 증분 빌드이며 최초 전체 빌드 시간이 아니다.
JSON parse·WorldSequences publisher와 `git diff --check`를 확인했다.

- [Product 빌드 결과](C:/Users/user/Desktop/LostArk/out/BuildPipeline/runs/20260909T142714591Z-debug-product.json)
- [원본 skin 대응](C:/Users/user/Desktop/LostArk/out/OddDollRestore20260909/source_skin_repair.json)
- [실제 CModel 검사](C:/Users/user/Desktop/LostArk/out/OddDollRestore20260909/model_probe_result.json)
- [WorldSequences 배포](C:/Users/user/Desktop/LostArk/out/OddDollRestore20260909/worldsequence_publish.log)
- [Resources 목록](C:/Users/user/Desktop/LostArk/out/OddDollRestore20260909/installed_resources.json)
- [최종 파일 점검](C:/Users/user/Desktop/LostArk/out/OddDollRestore20260909/final_checkpoint.json)

Client/UI 실행·캡처·육안 판정은 하지 않았다. 마지막 확인에서 이 PC는 LAN server-host이고 Client/Server는 꺼져 있었다.
Visual Studio `Debug | x64`, `Server + Client` profile → `Ctrl+F5`로 실행한다.
Lobby → KoukuSaydon → F1의 Object Tool에서 인형/Motion을 선택한다. Action Composition Resources의 World에도 같은 항목이 연결된다.
Rendering Workbench에서 카메라·설정을 유지하고 `Capture A: legacy` 다음 `Capture B: recovered`를 실행한다.
