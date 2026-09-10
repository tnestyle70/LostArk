# 2026-09-10 워로드 얼굴 program 12 조사 RESULT

`feature/customizing-finish`. main(`c67a47b2`)을 리베이스한 뒤 워로드 얼굴이 검게 나오고
피부색 조절도 닿지 않는다는 보고를 받고 조사했다. **결론부터 적으면 데이터 쪽 결함이
아니다.** 카탈로그 행, 텍스처 레지스터, 파라미터, 모델 게이트, 셰이더 컴파일까지 전부
정상이며 `source.character.classic-head-legacy.v1`(source program 12)의 출력만 검다.

## 1. 무엇이 바뀌었나

09-09까지 워로드 얼굴은 `source override 0`이었고 일반 셰이더가 바디에 구워진
`pc_wr_00_face_d.tga` / `_face_s.tga`를 소비했다. main `c67a47b2`가 처음으로
`Warlord.wmodel / pc_wr_face_mi` 행을 `classic-head-legacy.v1`로 추가했다.
그 행의 텍스처 다섯 개는 전부 stub이었다.

```
reg 0  efmaster_material_prologue/normal.tga
reg 1  efmaster_material_prologue/null.tga        <- 디퓨즈 자리
reg 2  efmaster_material_prologue/null.tga
reg 3  efmaster_material_prologue/flat_red.tga
reg 4  efmaster_material_prologue/statefx_default.tga
```

디퓨즈가 `null.tga`이므로 검은 얼굴은 당연한 결과였다. 이 다섯은 원작 base material
`ch.pc_oldmaterial.pc_head_opa`의 `ReferencedTextures` 기본값 그대로이며, 실제 얼굴
텍스처를 넣은 상태로 그려본 흔적이 아니다.

## 2. 실물 텍스처를 넣었다 — 그래도 검다

`pc_wr_face_mi`의 원작 체인을 umodel로 추출해 레지스터 의미를 확정했다.

```
pc_wr_face_mi -> mastermaterial_ch.pc_oldmaterial.pc_wr_head_opa -> Material3 ch.pc_oldmaterial.pc_head_opa
```

부모 `pc_wr_head_opa`가 선언하는 텍스처는 세 개다.

| 파라미터 | 값 |
|---|---|
| `var_headbase_meshtype_diffuse` | `pc_wr_00_face_d` |
| `var_headbase_meshtype_overlay` | `pc_wr_00_face_n` |
| `var_headbase_meshtype_specular` | `pc_wr_00_face_s` |

base material의 `ReferencedTextures` 순서 `[normal, null, flat_red, statefx_default]`와
`CollectedTextureParameters`(specular=flat_red, diffuse=null, decal=null)를 대조하면
레지스터 의미가 나온다. 셰이더 코드와도 일치한다.

| reg | 의미 | Base12 근거 |
|---:|---|---|
| 0 | 노멀 | `t0.Sample(v4.xy).xy`를 `*2-1`한 뒤 z 복원 |
| 1 | 디퓨즈 | `t1.Sample(v4.xy).xyz` |
| 2 | decal | `decal_region`으로 변환한 UV로 샘플 |
| 3 | 스페큘러 | `t3.Sample(v4.xy)`의 `.y`를 블렌드 가중치로 |
| 4 | statefx | `state`/`state_noise` 경로 |

그래서 0/1/3에 `pc_wr_00_face_n.dds`(ATI2) / `_face_d.tga` / `_face_s.tga`를 넣고
빌드했다. **화면은 그대로 검고 피부색도 닿지 않는다.**

## 3. 데이터·런타임 쪽은 전부 정상으로 확인했다

| 확인 | 방법 | 결과 |
|---|---|---|
| 파라미터 집합 | 패밀리의 `parameter("...")` 호출 이름을 추출해 행의 키와 대조 | 23개 요구 / 23개 보유, 누락·잉여 0 → `Configure` 성공 |
| 모델 게이트 | `Model.cpp`의 `failOverride`는 `E_INVALIDARG`로 **모델 전체**를 실패시킨다 | 워로드 몸이 화면에 있으므로 얼굴 행도 통과했다 |
| program 번호 | `Model.cpp` 게이트는 `program > 14u`만 거부 | 12는 통과 |
| 텍스처 실물 | `_face_d.tga` 평균 RGB (153,126,118) — 창술사 얼굴 (162,126,117)과 같은 살색 | 정상 |
| TGA 로더 | `Material.cpp`의 디코더가 imageType 2와 10(RLE) 모두 처리 | `_face_s.tga`가 RLE지만 문제 없음 |
| 재질 이름 | `Warlord.wmodel`이 `pc_wr_face_mi`를 실제로 가진다 | 일치 |
| 셰이더 배포 | Engine / EngineSDK / Client 세 사본 SHA-256 동일, `case 12u` 존재 | 동일 |
| 컴파일 | `Shader_VtxAnimMeshBinary.cso`(12:18) > HLSLI(11:58) | program 12가 CSO에 들어 있다 |

## 4. 셰이더 추적

`Shader_SourceCharacterMaterial.hlsli`가 `output.diffuse = native.targets[3].rgb`를 쓰고
`Shader_VtxAnimMeshBinary.hlsl`이 `output.vEmissive = source.indirect`(= `targets[0]`)를 쓴다.

Base12에서 `targets[3]`은 디퓨즈 체인이다.

```
27  r2.xyz = t1.Sample(faceUV).xyz            // 얼굴 디퓨즈
28  r1.xyz = -r2 * cb0[5] + r1                // cb0[5] = var_base_skincolor_ui
29  r3.xyz = r2 * cb0[5]
30  r1.xyz = r1.w * r1 + r3                   // decal alpha로 lerp
33  r1.xyz = r1.w * r2 + r1                   // t3.y로 lerp
34~35                                          // state 경로, state=(0,0,0,0)이라 통과
63~65 r0.xyz = r0 * cb0[12] * 1               // cb0[12] = globaldiffuseintensity = 1
104 o3.xyz = r0.xyz
```

명령 35부터 104 사이에 `r0.xyz`를 덮는 명령은 63/64/65뿐이고 셋 다 항등이다. 즉 G-buffer
디퓨즈는 얼굴 텍스처여야 한다. 상수 배치도 레지스터 단위로 맞다.

| 레지스터 | 패밀리가 넣는 값 | Base12가 쓰는 용도 | 일치 |
|---:|---|---|---|
| 2 / 3 / 4 | selectioncolor / hit_color / buffcolor | 명령 84~87 rim·flash 합성 | O |
| 5 | `var_base_skincolor_ui` | 디퓨즈 곱 | O |
| 6 | `decal_region` | decal UV 변환 | O |
| 7 / 9 | `state` / `state_noise` | state 경로 | O |
| 12 | `globaldiffuseintensity` | 디퓨즈 스케일 | O |
| 8 / 10 / 11 / 13 | 런타임이 시간값으로 덮어씀 | 파형 | O |

## 5. 남은 차이와 한계

Base12는 `source[14]/[15]/[16]`을 hemisphere ambient(하늘색·바닥색·강도)로 읽는데
패밀리는 `[13]`까지만 채운다. 다만 **이건 워로드만의 문제가 아니다.** 정상으로 보이는
program 4(classic-head)도 같은 구조를 `source[39]/[40]/[41]`에서 읽고 패밀리는 `[38]`까지만
채운다. 팀장이 3/8/9에 대해 기록한 "scene cube/SH 입력 미연결"과 같은 자리이며 네 클래스
공통이므로 워로드만 검은 이유로는 설명되지 않는다.

`Light12`도 상수 0~21을 연속으로 읽고 런타임이 6/8/9/15/16.x/20/21을 주입한다. 패밀리가
채우는 2~19와 겹침이 맞아떨어져 구조적 어긋남은 찾지 못했다.

**따라서 남은 후보는 생성된 program 12 코드 자체 또는 deferred 통합이며, 그 이상은 이번
조사에서 확정하지 못했다.** program 12는 워로드 얼굴 한 슬롯만 쓰는 패밀리이고 stub
텍스처로 커밋됐으므로 실제 텍스처로 화면에 그려진 적이 없다.

## 6. 이번 브랜치에서 한 일과 하지 않은 일

- 했다: `Data/Actors/CharacterCatalog.json`의 해당 행 텍스처 세 개를 stub에서 실물로 교체.
  레지스터 배치를 위 근거로 확정했으므로 program 12가 고쳐지면 텍스처는 맞게 들어가 있다.
- 하지 않았다: 상수를 임의로 채우거나 program 4 + `pc_wr_face_mi_high` 체인으로 바꾸는 것.
  후자는 이전에 시도해 얼굴이 하얗게 떴고 이유를
  `Tools/CharacterCustomizing/build_face_material_overrides.py`에 실측으로 남겨 두었다
  (`skin_metalicness_power` 5.0 대 0.5/1.0 등, 메시가 그리는 MIC와 다른 MIC의 값을 program 4에
  먹이는 구성).
- 하지 않았다: 행 제거. 사용자 판단으로 현재 상태를 유지하고 팀장 작업 완료 후 재확인하기로 했다.

## 7. 사용자 화면 확인

검은 얼굴은 사용자가 실제 Client에서 관찰한 상태다. 이 문서의 수치·추적은 전부 정적
검사이며 화면 판정은 사용자가 한다. 수정 완료로 기록하지 않는다.
