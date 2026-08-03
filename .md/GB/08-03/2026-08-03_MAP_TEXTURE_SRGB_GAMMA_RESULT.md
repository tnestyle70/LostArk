# 맵 텍스처 sRGB 디코드 누락 수정 결과

작성일: 2026-08-03
작업 브랜치: `CY`
대상 증상: 발탄 맵의 나무·바닥 등이 텍스처가 안 입혀진 것처럼 뿌옇고 하얗게 보임

## 1. 결론

텍스처 파이프라인에는 결손이 없었다. 원인은 sRGB로 저장된 색상 텍스처를 선형 공간
파이프라인에 그대로 넣어 감마가 두 번 적용된 것이다. `Engine/Private/Material.cpp`의
텍스처 로드에서 색상 슬롯만 sRGB로 디코드하도록 고쳤다.

## 2. 기각한 가설과 실측 근거

`Client/Bin/DataFiles/Map/LV_LUT_HEARTRB_ED.mapassets`(카탈로그 v4, 자산 275)와
설치된 `Client/Bin/Resources/Map/LV_LUT_HEARTRB_ED` 실측이다.

| 가설 | 실측 | 판정 |
|---|---|---|
| 텍스처 파일 누락·경로 오류 | 실제 머티리얼 346개의 baseColor 채움률 100%, 런타임 해석 규칙으로 해결 실패 0 | 기각 |
| 머티리얼↔서브메시 매핑 오류 | 서브메시 350개 중 빈 머티리얼을 참조하는 것 0개 | 기각 |
| UV 파손 | 자산 275개 전부 UV 범위 정상, 축퇴 0 | 기각 |
| 텍스처 자체가 흰색 | 고유 diffuse 149장 평균 밝기 70~100/255, 대비 정상 | 기각 |
| 알파 컷아웃 오적용 | `PS_MAIN`의 `a < 0.3` discard는 유지되며 sRGB 포맷은 알파를 변환하지 않음 | 기각 |

레거시 `Resource/LostArk/` 접두는 `WMaterialReader.cpp:31`의 `ResolveBelowAssetRoot`가
이미 벗겨내므로 문제가 아니다.

## 3. 확정한 원인

렌더 파이프라인은 선형 공간 HDR로 설계돼 있다.

- 씬 타깃: `DXGI_FORMAT_R16G16B16A16_FLOAT` (`Engine/Private/Renderer.cpp`)
- 백버퍼: `DXGI_FORMAT_R8G8B8A8_UNORM` (`Engine/Private/Graphic_Device.cpp:190`)
- 최종 인코드: `Shader_Deferred.hlsl:372` `pow(saturate(vMapped), 1.f / 2.2f)`
  주석에도 "파이프라인 전체에서 감마는 이 한 줄이 전부다"라고 명시돼 있다.

그런데 `Material.cpp`의 `LoadTexture`가 `CreateDDSTextureFromFile`을 플래그 없이
호출해 sRGB DDS를 `BC1_UNORM`/`BC3_UNORM`으로 올렸다. `DDS_LOADER_FORCE_SRGB`는
Engine/Client 전체에서 한 번도 사용되지 않았다. 그 결과 셰이더가 sRGB 값을 선형 값으로
소비했다.

정량적으로, 텍스처 평균 밝기 0.353은 선형으로 0.10이어야 하는데 0.353이 그대로
들어가 약 3.5배의 에너지가 조명 계산에 투입됐다. 여기에 노출 배수 2.0과 블룸 0.8이
곱해지고 마지막에 감마가 적용되어, 빛을 정면으로 받는 곡면과 위를 향한 바닥이 먼저
포화되고 비스듬한 면만 디테일이 남는 화면이 만들어졌다.

## 4. 변경 내용

변경 파일은 `Engine/Private/Material.cpp` 하나이며 24줄 추가, 5줄 삭제다.
파일 인코딩(ASCII, BOM 없음)과 CRLF 줄바꿈을 그대로 유지했고 주석은 파일에 맞춰
영문으로 작성했다.

1. `IsColorTextureSlot(aiTextureType)` 추가 — `aiTextureType_DIFFUSE`와
   `aiTextureType_EMISSIVE`만 색상 슬롯으로 판정한다.
2. `LoadTexture`에 `isColorSlot` 인자 추가. DDS는 `CreateDDSTextureFromFileEx`로
   `DDS_LOADER_FORCE_SRGB`, WIC(PNG)는 `CreateWICTextureFromFileEx`로
   `WIC_LOADER_FORCE_SRGB`를 색상 슬롯에만 적용한다.
3. `LoadTgaTexture`에 같은 인자를 전달해 색상 슬롯일 때만
   `DXGI_FORMAT_R8G8B8A8_UNORM_SRGB`로 생성한다.
4. `AddTexture`가 슬롯 타입으로 판정해 전달한다.

노멀(ATI2/BC5 215장), 스페큘러, ORM, 러프니스, 메탈릭, 오파시티는 색이 아니라
데이터이므로 선형을 유지한다. `AddSolidTexture`의 1×1 회색 fallback은 디버그
표시용이라 변경하지 않았다.

## 5. 구현 상태와 검증 상태

- 구현 완료: 위 4절 변경.
- 자동 검증: 미실행. Engine 빌드와 정본 회귀는 아직 돌리지 않았다.
- 수동 검증: 미실행. 화면 확인 필요.
- 검증 방법: Engine x64 빌드 → `UpdateLib.bat Debug` → Client 빌드 후
  스크린샷 1~4와 같은 카메라 위치에서 재촬영해 비교한다.

## 6. 예상되는 부작용

- 이 변경은 Engine 공통 경로라 발탄뿐 아니라 베른과 캐릭터의 밝기도 함께 바뀐다.
  TGA는 캐릭터 218장, 베른 20장이 사용 중이라 함께 맞췄다.
- 현재의 노출 2.0과 블룸 0.8은 잘못된 입력에 맞춰 조정됐을 가능성이 높다. 수정 후
  전체가 어둡게 보이면 그 두 상수를 재조정해야 하며, 이는 별도 변경 단위로 다룬다.
- `CMaterial::Initialize(const aiMaterial*, ...)`의 Assimp/FBX 경로는 변경하지
  않았다. 런타임 리소스에 `.fbx`가 0개라 제품 경로가 아니기 때문이다.

## 7. 남은 문제 (이번 변경 범위 밖)

1. 렌더 프로파일 상수화. 카탈로그 275행이 예외 없이 `Opaque Back`이다. 생성기
   기본값(`build_maptool_scene.py:276`, `extract_ue3_landscape.py:2424`)이며 원본
   블렌드 모드가 추출된 적이 없다. 그 결과 구름판·물 4종·이끼안개·폭포거품 등 11개가
   불투명으로, 잎·풀·나무 계열 55개가 단면으로 그려진다.
2. 재질 파라미터 결손. 실제 머티리얼 346개 중 노멀 67%, 스페큘러 2개, ORM·러프니스·
   메탈릭 0개다. 스페큘러 텍스처가 없으면 셰이더가 상수 `g_SpecularIntensity`를 쓰므로
   재질과 무관하게 균일한 광택이 깔린다.
3. baseColor가 원본 Material Instance가 지정한 바로 그 텍스처인지는 대조하지 않았다.
4. 자산마다 이름 없는 빈 머티리얼 슬롯이 하나씩 존재한다(해시가 FNV-1a 빈 문자열 값).
   어떤 서브메시도 참조하지 않아 현재 무해하다.

## 8. 리소스 취급

`Client/Bin/Resources`와 `Client/Bin/DataFiles/Map`의 파일은 읽기만 했고 이동·수정·
생성하지 않았다. 리소스 팩 lock과 manifest도 변경 대상이 아니다.
