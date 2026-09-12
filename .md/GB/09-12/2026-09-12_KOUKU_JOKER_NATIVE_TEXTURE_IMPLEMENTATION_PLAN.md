# 쿠크 조커 카드 native texture 연결 구현 계획

## G00. 실제 결함과 범위

월드 오브젝트 `world.object.kouku.joker_card`는
`Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel`을 사용한다. 설치 WModel의
material slot 0 이름은 일반 카드와 같은 `mn_rhoc_00_mi`지만 내장 D/N/S는
`textures/mn_rhoc_00-1_{d,n,s}.dds`다. 현재 `Data/Actors/BossCatalog.json`의 이 모델
override는 일반 카드 MIC와 `mn_rhoc_00_{d,n,s}.dds`를 연결한다. native material
연결을 추가한 `359412c46d12fd5a0df3155045d6468a01acfd12`에 이 행이 들어갔다.

09-11 [보스·소품 재질 결과](../09-11/2026-09-11_KOUKU_BOSS_PROP_NATIVE_MATERIAL_IMPLEMENTATION_RESULT.md)는
여러 모델의 native 경로 도입과 당시 로드·바인딩 검증을 소유한다. 이번 계획은 사용자가
보고한 조커 앞면 누락에 대응하는 한 모델의 variant 입력 교정이다. 새 shader·생성기·모델
runtime을 만들지 않는다. 실제 사용자 화면 결과는 RESULT에서 별도로 기록한다.

## G01. 데이터 정본과 실제 소비

`CWorldSequencePlayer::Prepare_ObjectResources`는
`CActorCatalog::Build_ModelLoadDescription`으로 해당 모델의 descriptor를 만든다.
`CActorCatalog`는 `CProjectDataRoot`의 `Data/Actors/BossCatalog.json`을 읽고 모델의
Resources-relative asset ID로 override를 선택한다. `CModel`이 cooked materialName으로
slot을 찾아 native surface와 `sourceCharacterTextures`를 교체하고, `CMaterial`이 이를
`g_SourceCharacterTexture0..6`에 바인딩한다.

family `source.character.monster-6ff78ae19259.v1`의 program 26은 expression 1의 diffuse
RGBA를 읽고 alpha가 약 1/3 미만이면 pixel을 discard한다. 일반 diffuse SRV를 따로 바꾸어도
이 native slot을 교체하지 못한다. 원본 조커 MIC와 일반 MIC의 native pixel shader와
36개 named parameter는 같다. 따라서 family, parameter, cooked materialName, 공통
texture 3..6을 유지하고 sourceMaterial과 texture 0..2만 variant로 맞춘다.

정본은 기존 BossCatalog의 authored 모델 행이다. 현재 추적된 도구에는 이 행을 재생성하는
조커 전용 writer가 없다. Effect native 생성기는 별도 Effect profile의 소유자다.
기존 Character variant D/N/S가 설치돼 있고 같은 원본을 사용하는 Effect 복원본 D/N/S와
byte hash가 일치하므로 Resources 복사·추출·재생성은 필요 없다.

## G02. 변경 파일과 교체 블록

| 파일 | 변경 |
|---|---|
| `C:/Users/user/Desktop/LostArk/Data/Actors/BossCatalog.json` | 조커 모델 행의 sourceMaterial 및 texture 0..2만 교정 |
| `C:/Users/user/Desktop/LostArk/.md/GB/렌더링이펙트복원V2.md` | cooked materialName과 variant native texture 입력의 반복 결함 기록 |
| `C:/Users/user/Desktop/LostArk/.md/GB/gotchas.md` | 같은 이름의 variant를 일반 카드 texture로 덮어쓰지 않는 확인 기준 |
| `C:/Users/user/Desktop/LostArk/.md/GB/09-12/2026-09-12_KOUKU_JOKER_NATIVE_TEXTURE_RESULT.md` | 실제 diff, 데이터·slot 검증과 사용자 화면 미확인 경계 |

정확한 선택 기준은 `modelAssetId`가
`Character/KoukuSaton/MN_RHOC_00-1/MN_RHOC_00-1.wmodel`인 한 행이다.
해당 행의 `sourceMaterial`을 다음 값으로 교체한다.

```json
"sourceMaterial": "mn_rhoc_00-1.mat.mn_rhoc_00-1_mi"
```

같은 행의 `textures` 배열을 다음 완전한 블록으로 교체한다. expression index와 colorSpace는
기존 native ABI를 유지한다. materialName은 WModel slot 검색용이므로 `mn_rhoc_00_mi`를 유지한다.

```json
"textures": [
  {
    "expressionIndex": 0,
    "assetId": "Character/KoukuSaton/MN_RHOC_00-1/textures/mn_rhoc_00-1_n.dds",
    "colorSpace": "linear"
  },
  {
    "expressionIndex": 1,
    "assetId": "Character/KoukuSaton/MN_RHOC_00-1/textures/mn_rhoc_00-1_d.dds",
    "colorSpace": "srgb"
  },
  {
    "expressionIndex": 2,
    "assetId": "Character/KoukuSaton/MN_RHOC_00-1/textures/mn_rhoc_00-1_s.dds",
    "colorSpace": "srgb"
  },
  {
    "expressionIndex": 3,
    "assetId": "Character/SourceMaterials/Kouku/efmaster_material_prologue/hdr07_1.dds",
    "colorSpace": "srgb"
  },
  {
    "expressionIndex": 4,
    "assetId": "Character/SourceMaterials/Kouku/efmaster_material_prologue/flat_black.dds",
    "colorSpace": "srgb"
  },
  {
    "expressionIndex": 5,
    "assetId": "Character/SourceMaterials/Kouku/efmaster_material_prologue/statefx_default.dds",
    "colorSpace": "srgb"
  },
  {
    "expressionIndex": 6,
    "assetId": "Character/SourceMaterials/Kouku/efmaster_material_prologue/brdf_beckmann_spec.dds",
    "colorSpace": "linear"
  }
]
```

H/CPP·schema·public interface는 바뀌지 않는다. 기존 Data 파일을 수정하므로 project/filter
추가 등록과 publisher 출력 교체가 없다. 카탈로그 초기화·모델 prototype은 process에 유지되므로
사용자는 Client를 재시작해 변경된 입력을 읽는다.

## G03. 적용 순서와 검증

계획을 작성한 뒤 위 모델의 네 문자열 필드만 교체한다. JSON parse, HEAD 대비 semantic diff의
정확한 네 leaf, 7개 texture의 Resources 상대 경로·존재·색 공간, 실제 WModel materialName과
내장 D/N/S 대응을 검사한다. 조커 Effect 원본 재질의 36개 parameter 및 native shader identity와
비교하고, `git diff --check`를 실행한다. 별도 validator나 harness를 추가하지 않는다.

ActorCatalog → CModel → CMaterial → program 26의 실제 소스 소비를 확인한다. 기존 headless
실행 검증 도구가 설치돼 있지 않으면 이를 C++ 실행 검증으로 기록하지 않는다. 이번 변경은 JSON만
바꾸므로 별도 Client·Engine 빌드나 DLL 배포를 하지 않으며 통합 담당이 진행 중인 빌드와 충돌하지
않는다. 에이전트는 Client/UI를 실행·조작·캡처하지 않는다.

사용자 확인 경로는 Client 재시작 → KoukuSaydon 아레나 → F1 → Object Tool →
`world.object.kouku.joker_card`(조커카드) → `Preview Default`다. 이어 조커 찾기 패턴에서 실제 조커 앞면을
확인한다. 정상 texture 연결과 화면에서의 최종 그림·색·alpha 확인을 구분한다.
