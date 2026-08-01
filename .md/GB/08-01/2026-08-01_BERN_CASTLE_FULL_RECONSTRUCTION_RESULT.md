# 베른성 전체 맵 복원 1차 구현 결과

작성일: 2026-08-01  
대상 area: `LV_BER_BERNCASTLE`  
런타임 경로: `CModel -> CMaterial`

## 1. 결론

베른성의 원본 정적 메시, Landscape, 원본 Foliage 개별 인스턴스를 MapTool에서 한
번에 불러오는 1차 런타임을 완료했다.

- 원본 exact StaticMesh: 950/950 추출 및 `.wmodel` Cook
- 원본 정적 배치: 32,324/32,324
- Landscape: 42/42
- Foliage native instance: 17,651/17,651
- MapTool 통합 배치: 50,017개
- 물리 catalog: BASE, LANDSCAPE, SL00~SL10의 13개 shard
- 고유 runtime asset: 1,003개
- 가장 큰 shard catalog: 422개(제한 512 이하)
- 실제 Debug Client에서 catalog READY, Objects 50,017 및 장면 렌더 확인

이 결과는 정적 형상, Landscape, 개별 수풀 배치와 현재 연결 가능한 재질·텍스처의
복원이다. 데칼 투영, 물 전용 셰이더, 원본 ParticleSystem, 조명, 충돌과 Navigation까지
완료됐다는 뜻은 아니다.

## 2. StaticMesh exact 추출과 Cook

기존 전체 export corpus는 베른성 950종 중 exact package/object 일치가 329종뿐이었다.
이름은 같지만 출처 package가 다른 20종과 미확보 601종이 있었으므로, 기존 결과를
재사용하지 않고 950종 전부를 원본 UPK에서 `-obj=<exact object>`로 다시 추출했다.

결과:

| 항목 | 결과 |
|---|---:|
| exact source export | 950/950 |
| `.wmodel` Cook | 950/950 |
| MaterialInstance 속성 | 1,397 |
| diffuse 연결 | 1,359 |
| normal 연결 | 1,253 |
| specular 연결 | 702 |
| emissive 연결 | 73 |
| 역할을 확정할 수 없는 material entry | 34 |

역할을 확정할 수 없는 texture는 파일명으로 추측 연결하지 않았다. 각 asset은 source와
runtime receipt, SHA-256, converter info를 보존한다.

주요 출력:

```text
C:\LostArkExtract\bern_full\manifests\bern_castle_assets.json
C:\LostArkExtract\bern_full\manifests\bern_castle_runtime_assets.json
C:\LostArkExtract\bern_full\source\<assetId>\
C:\LostArkExtract\bern_full\runtime\<assetId>\
```

## 3. MapTool shard 계약

기존 catalog는 asset 512개 제한이 있어 950종과 Landscape 42종을 한 파일에 넣을 수
없었다. 이를 위해 `LOSTARK_MAP_SHARD_SET 1` 포맷을 추가했다.

```text
LV_BER_BERNCASTLE.mapset
  BASE
  LANDSCAPE
  SL00 ... SL10
```

런타임 로더는 다음을 모두 검증한 뒤 한 번만 commit한다.

- 상대 파일명과 확장자, root 이탈 여부
- shard/asset/placement 선언 수
- child area ID
- asset 중복 정의의 전체 필드 동일성
- prototype tag 충돌
- shard 전체 placement ID 중복
- 모든 placement의 catalog join

어느 shard에서든 실패하면 신규 stage를 버리고 기존 Scene을 보존한다. 50,017개
원본 대량 배치를 실수로 단일 파일에 다시 저장하지 않도록 shard mode의 Save는
의도적으로 비활성화했다.

Loader에서도 부분 등록을 남기지 않도록 ASSET_TEST resource rollback scope를
추가했다. Shader/Model `Create()`가 `nullptr`를 반환하면 Prototype Manager가 이를
거부하고, Loader는 그 시점까지 등록한 ASSET_TEST object/prototype 전체를 정리한다.
따라서 손상된 1개 모델 때문에 같은 프로세스의 재시도가 tag 충돌로 계속 실패하지
않는다. 베른성에 선택적 DeployProp 문서가 둘 다 없을 때는 정상적인 empty lane으로
처리하며 Map catalog 성공 상태를 오류 문구로 덮어쓰지 않는다.

DataFiles:

```text
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE.mapset
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_BASE.*
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_LANDSCAPE.*
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_SL00.*
...
Client/Bin/DataFiles/Map/LV_BER_BERNCASTLE_SL10.*
```

## 4. 런타임 리소스

```text
Client/Bin/Resources/LostArk/Map/LV_BER_BERNCASTLE/
  961 asset pack / 961 WModel
  (기본 StaticMesh 950 + Foliage supplemental 11)

Client/Bin/Resources/LostArk/Map/LV_BER_BERNCASTLE_T/Landscape/
  42 WModel
```

정적 pack은 약 1.78 GiB다. 정적 texture 3,315개를 content hash로 감사한 결과
고유 내용은 1,354개였고, 약 1.04 GiB가 asset별 중복 복사다. 전체 베른성을 한 번에
로딩한 Debug Client의 private memory는 Foliage 연결 전 약 3.58 GiB, 연결 후 약
3.82 GB였다. 다음 성능 작업은 texture를
content-address 방식으로 한 번만 보관하고, streaming level별로 prototype을 지연
로딩·해제하는 것이다.

`Client/Bin/Resources`는 Git에 포함되지 않는 팀 공유 리소스다. 다른 팀원이
DataFiles만 pull하면 모델이 나타나지 않으므로 위 두 runtime pack도 함께 받아야 한다.

## 5. 비정적 원본 inventory

22개 core level UPK를 직접 읽어 정적 메시 밖의 원본 항목을 별도 manifest로
고정했다.

| 종류 | 원본 항목 |
|---|---:|
| Decal | 86 |
| Foliage component | 1,697 |
| Water placement | 108 |
| ParticleSystemComponent | 1,373 |
| Light | 326 |
| Fog | 1 |
| Wind | 1 |
| 합계 | 3,592 |

추가로 decal material 11종, foliage mesh 15종, ParticleSystem 109종, water asset
15종을 확인했다. 원본 serial offset/size/hash, actor/component, transform evidence,
object reference를 보존한다.

```text
C:\LostArkExtract\bern_full\manifests\bern_castle_nonstatic.json
```

`CachedParentToWorld`의 Lost Ark UE3 Matrix는 마지막 4개 float가
`[homogeneous, X, Y, Z]` 순서다. 초기 추출기가 이를 `[X, Y, Z, homogeneous]`로
해석해 323개 Light 위치의 X가 모두 `1`이 되는 오류가 있었으므로 추출기와 회귀
테스트를 수정하고 manifest를 다시 생성했다. 수정 후 첫 Point Light 위치는
`(17271.6992, 16020.8789, 5333.4541)`이며, 갱신된 manifest SHA-256은
`D02335E0C0CD865C65C02E26F04837D862CD9D4A75C54A0EB2BA18CAC7788FAF`다.

현재 950 정적 runtime 목록에 없던 foliage mesh 11종은 별도 supplemental pack으로
exact 추출과 Cook을 완료했다. 1,407개 component 사용 근거, material 11개, texture
15개, 명시적 texture role 17개를 보존했으며 11개 WModel은 모두 `WINT`다.

```text
C:\LostArkExtract\bern_full\foliage\manifests\
  bern_castle_foliage_supplement_assets.json
  bern_castle_foliage_supplement_runtime_assets.json
```

foliage의 native suffix를 원본 UPK에서 다시 열어 엄격하게 해독했다. 각 레코드는
`FMatrix 16 float + lightmap/shadowmap UV bias 4 float`의 80바이트이며, component
1,697개에서 instance 17,651개를 복구했다. 원본 package/serial/suffix SHA-256,
ShadowMap reference, 행렬과 UV bias를 manifest에 보존하고 Client 좌표계로 변환했다.

| 항목 | 결과 |
|---|---:|
| Foliage component | 1,697 |
| 개별 instance | 17,651 |
| 기존 950에서 재사용한 mesh | 4종 / 938 instance |
| supplemental mesh | 11종 / 16,713 instance |
| 음수 scale / reflection | 0 / 0 |

```text
C:\LostArkExtract\bern_full\manifests\bern_castle_foliage_overlay.json
```

overlay placement는 원본 `sourceLevel`을 기준으로 BASE와 SL00~SL10에 분배한다. 기존
4개 asset ID는 중복 정의하지 않고 재사용하고, supplemental 11개만 catalog에 추가한다.
low-domain 안정 ID, source/runtime ID 충돌, shard별 asset join과 전체 count를 검증한
뒤 13개 shard를 원자 교체한다.

첫 Decal fixture도 원본 연결을 복구했다. 아직 런타임 투영은 적용하지 않았다.

```text
DecalComponent
-> lv_decal_01.mat.lv_common_decal_05_mi
-> efbasematerial_lv_prologue.decals.decal_translucent
```

- diffuse: `lv_common_decal_05_d.dds`, 512x512 DXT5
- conditional opacity texture: `fx_tex_a_02710.dds`, 512x512 DXT1
- 실제 선택 branch: `1.use_opacity_texture=false`, diffuse alpha 사용
- 산출: `C:\LostArkExtract\bern_full\decal_probe`
- 안전 계약: `runtimeMutation=false`, `renderProfileGenerated=false`

86개 Decal의 위치·방향·크기는 확보됐지만 나머지 10개 material family의 parent/texture
그래프와 projected decal renderer는 아직 후속 작업이다.

## 6. Navigation과 충돌 경계

기존 ASSET_TEST가 `ValtanArena.navgrid`를 고정 사용하던 문제를 제거하고 area별 계약을
추가했다.

```text
<area>.navsource
<area>.navpaint
<area>.navgrid
<area>.navblockers
Prototype_Component_Navigation_<area>
```

기존 `LV_LUT_HEARTRB_ED`만 호환을 위해 `ValtanArena.*`를 계속 사용한다. 베른성
`.navgrid`가 없으면 ASSET_TEST 진입을 실패시키지 않고 Character와 Valtan을 Navigation
없이 만든다. MapTool은 베른성 전용 Nav Bounds/Bake 시작 상태를 표시한다.

최종 회귀 실행에서 Windows의 `std::filesystem::status`가 존재하지 않는 Bern
`.navgrid`에 `FILE_NOT_FOUND`를 반환해 Loader가 실패하는 문제가 확인됐다. 이제
`FILE_NOT_FOUND`와 `PATH_NOT_FOUND`는 정상적인 `bootstrap pending`으로 처리하고,
존재하는 경로가 일반 파일이 아니거나 권한·I/O 오류인 경우에만 실패한다. Loader의
실패 모달은 현재 단계와 모델 Asset ID를 표시하며, 부분 등록은 rollback된다.

아직 완료되지 않은 항목:

- Landscape와 길·계단·다리를 WALKABLE로 구분하는 source role
- 지붕·나무·장식물을 보행면에서 제외하는 규칙
- 베른성 `.navgrid` 실제 bake 및 검증
- 원본 Landscape hole mask
- StaticMesh BodySetup/AggGeom 또는 별도 blocker collision

모든 Opaque mesh의 최고 표면을 바닥으로 bake하면 지붕과 나무 위까지 걸을 수 있게
되므로 그 방식은 사용하지 않는다.

## 7. 실행 방법

`Client/Bin/DataFiles/Map/ACTIVE.maparea`의 선택 줄을 임시로 다음처럼 바꾼다.

```text
LOSTARK_MAP_AREA_SELECTION 1 "LV_BER_BERNCASTLE"
```

Debug Client를 실행하고 ASSET_TEST 로딩을 완료한 뒤 F1로 MapTool을 연다. 정상 기준은
다음과 같다.

```text
Catalog: READY
Objects: 50017
BG_EVENT_A (950)
Bern Castle Native Foliage (11)
Bern Castle Landscape (42)
Shard set is read-only
```

검증이 끝나면 팀의 현재 기본값인 `LV_LUT_HEARTRB_ED`로 되돌린다. 이번 작업에서도
기본 선택 파일은 원래 값으로 복원했다.

## 8. 검증 결과

- Bern static/non-static/foliage/decal pipeline unit test: 26개 통과
- Map scene compiler/shard builder unit test: 21개 통과
- Python `py_compile`: 통과
- Engine x64 Debug/Release: 통과
- UpdateLib Debug/Release: 통과
- Client x64 Debug/Release: 통과
- 실제 Debug Client: 13 shards / 1,003 assets / 50,017 objects 로드 및 렌더 확인
- `.navgrid`가 없는 Bern 선택으로 최종 Debug Client 재실행: ASSET_TEST 진입 및
  정적 배치+Landscape+Foliage 렌더 확인, private memory 약 3.82 GB, 정상 종료 코드 0
- `git diff --check`: 통과

최종 렌더 캡처:

```text
C:\Users\USER\.codex\visualizations\2026\07\29\
019fabb0-ba28-7791-b24d-2b682de4be88\bern_final_runtime_scene.png
C:\Users\USER\.codex\visualizations\2026\07\29\
019fabb0-ba28-7791-b24d-2b682de4be88\bern_foliage_maptool_status.png
```

## 9. 다음 구현 순서

1. texture 중복 제거와 streaming-level 지연 로딩
2. 순수 단일 translucent water asset 1종으로 물 material 경로 검증 후 material-slot 단위로 확장
3. 검증한 첫 fixture로 decal projection을 구현한 뒤 11 material family로 확장
4. ParticleSystem 109종을 EffectAsset/Effect_Tool 변환 입력에 연결
5. light lifecycle/proxy, fog, wind
6. `SURFACE/STATIC_BLOCKER/RUNTIME_BLOCKER/IGNORE/REVIEW` Navigation role sidecar
7. Landscape 1조각 pilot bake, hole mask 반영 후 본성 영역 `.navgrid` 확장
8. BodySetup/AggGeom을 확보하거나 수동 검증한 blocker를 게임 collision에 연결

각 단계는 source manifest와 runtime 구현 상태를 분리해 기록한다.
