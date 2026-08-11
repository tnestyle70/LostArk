# 캐릭터 염색(dye) 런타임 — 구현 완료

작성자: JS · 2026-08-10 밤 · 브랜치 `feature/warlord-cloth-bone-chain`

08-07 조사(`../08-07/2026-08-07_LOSTARK_CHARACTER_DYE_COLOR_MASK_RESULT.md`)에서 확정한
"색 영역 마스크 + 영역별 틴트" 경로를 구현했다. 차원술사·도화가의 흰옷과 창술사의
빠진 색이 원본 파라미터로 복원됐고, 워로드는 픽셀 단위로 이전과 동일함을 확인했다.

## 1. 구조 — 재쿠킹 없는 패치 경로

ModelAssetConverter는 exe만 있고 소스가 없다. 그래서 컨버터를 고치는 대신
**wmodel의 MATERIAL 섹션만 WMA2→WMA3로 다시 쓰는 패치 도구**를 만들었다. 지오메트리·
스켈레톤·애니메이션 섹션은 바이트 단위로 그대로고, FBX 왕복도 없다.

```text
umodel props.txt ──extract──▶ DyeTints/<Class>.json ──apply──▶ wmodel(WMA3)
                                                              + textures/*_cm*
런타임: WMaterialReader(V1/V2/V3) → MODEL_COLOR_TINT → CMaterial(BASE_COLOR 슬롯)
        → Bind_DeferredMaterialInputs → Shader_Vtx(Anim)MeshBinary 합성
```

| 계층 | 파일 | 내용 |
|---|---|---|
| 포맷 | `Winters/WFormatTypes.h` | `WMA3` 매직, `MATERIAL_ENTRY_V3`(V2 + colorMaskPath + float4 틴트 4개, 5340B) |
| 리더 | `Winters/WMaterialReader.cpp` | V3 분기. V1/V2는 그대로 identity로 읽힌다 |
| 데이터 | `BinaryAsset/ModelAssetData.h` | `MODEL_COLOR_TINT`(isEnabled + diffuse/a/b/c) |
| 머티리얼 | `Material.h/.cpp` | 마스크를 `aiTextureType_BASE_COLOR` 슬롯에 로드(비 sRGB), 틴트 보관 |
| 모델 | `Model.h/.cpp` | `Get_MaterialColorTint(iMeshIndex)` |
| 바인딩 | `Client/DeferredMaterialRenderUtils.cpp` | `g_HasDyeMask` + 마스크/틴트 4색. 플래그는 실패 허용으로 매 메시 바인딩(염색 계약 없는 셰이더 통과 + 이전 메시 상태 잔류 방지) |
| 셰이더 | `Shader_VtxAnimMeshBinary.hlsl`, `Shader_VtxMeshBinary.hlsl` | `tint = diffusecolor × lerp(1,a,mask.r) × lerp(1,b,mask.g) × lerp(1,c,mask.b)` |

도구: `Tools/ModelAssetConverter/patch_wmodel_dye.py` — `list`/`extract`/`apply` 3모드,
apply는 멱등이고 최초 1회 `.wmodel.bak`을 남기며, 참조한 마스크 파일이 wmodel 옆에
없으면 거부한다. 틴트 정본은 `Tools/ModelAssetConverter/DyeTints/<Class>.json`이다
(umodel MaterialInstanceConstant 덤프에서 추출한 원본 수치 — 밸런스 provenance
receipt와 같은 성격의 원본 근거 데이터).

## 2. 클래스별 적용

| 클래스 | 염색된 머티리얼 | 주의점 |
|---|---|---|
| 도화가 | 장비 7파츠 전부 (upper/upper1/lower/lower1/arm/shoulder/helmet/helmet1) | 몸체는 얼굴·눈·머리만 보여 대상 없음 |
| 창술사 | 장비 5파츠 | **wmodel 머티리얼명은 기본형인데 배포 텍스처는 upper/lower만 `-1` 변종.** props 선택은 각 변종의 `texture_diffuse`가 배포 `_d`와 일치하는 것으로 판정했다 |
| 차원술사 | combined body의 의상 5머티리얼 | `PC_SP_M_01`을 umodel로 신규 추출. 신세대 `realpbr_avatarparts_msk_v2`는 전체 곱 색 이름이 `basecolor_color`다(extract가 별칭 처리). `diffusecolor_d`(4번째 영역)도 갖고 있으나 마스크 알파가 사실상 전부 255라 이 에셋들에선 미사용 |
| 워로드 | 패치 안 함 | 틴트가 항등이라 불필요(08-07 3절). identity 경로 무변화 검증용 대조군 |

## 3. "아직 흰 부위"는 원작 색이다 — 실측 근거

- 차원술사 바지(`lower01`): 디퓨즈 평균 (200,200,200) × 영역 A 틴트 0.906 회백.
  마스크는 전면이 영역 A(평균 R=255)라 누락 영역 없음.
- 도화가 허리띠(`upper1`): 틴트 전부 (1,1,1) + 디퓨즈 평균 (151,150,150) 무채색 +
  마스크 36%가 무영역 — 원작도 이 조각은 디퓨즈 원색으로 그린다.
- 마스크 채널 통계는 scratchpad `mask_stats.py`(TGA 직접 디코드)로 냈다. 커밋 안 함.

## 4. 근사와 열린 항목

원본 부모 머티리얼은 노드 917개 그래프라 정확 복원이 불가능해(08-07 6절) 합성식은
영역별 lerp-곱 근사다. 현재 무시하는 것: 영역 틴트의 알파(강도 추정), `diffusecolor_d`
(마스크 알파 영역 — 현 에셋 미사용), `_m` variation 마스크(`mask_variation_visible=0`).
특정 부위 색이 원작과 어긋나면 이 셋 순서로 의심한다.

## 5. 팀 인계 — 배포 순서 결합

- 리더는 하위 호환이라 **코드만 받으면 아무 영향 없다** (기존 V2 wmodel은 지금처럼
  렌더). pull 후 `UpdateLib.bat` 필수(Engine 공개 헤더 3개 변경).
- **위험 조합은 "V3 wmodel + 옛 Engine.dll" 하나**다. 옛 리더가 WMA3 매직을 거부해
  캐릭터 로드가 실패한다. 공유 드라이브의 Character 폴더(Artist/LanceMaster/
  DimensionMaster 3개, `.bak` 제외)는 이 브랜치가 main에 머지된 뒤에 올리고, "코드
  먼저 + UpdateLib 후 폴더 교체"를 공지에 적는다.
- 새로 쿠킹하는 wmodel은 여전히 V2로 나온다(컨버터 무수정). 필요하면 쿠킹 후
  `patch_wmodel_dye.py apply`를 한 번 더 돌린다.

## 6. 검증

```text
Engine/Client Debug 빌드         통과 (UpdateLib 포함)
실행 확인 (작성자+사용자 육안)    차원술사·도화가·창술사 색 복원, 워로드 무변화,
                                맵·NPC·이펙트 기존 렌더 유지
정본 회귀                        커밋 전 실행 (결과는 커밋 메시지에)
```
