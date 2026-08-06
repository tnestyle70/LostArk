# Effect Tool 클래스별 리소스 저작 구현 계획

## 현재 실측과 목표

`Client/Bin/Resources/Effect`에는 다음 다섯 저작 도메인이 실제 물리 폴더로 존재한다.

| 도메인 | WModel | DDS | 합계 |
|---|---:|---:|---:|
| Artist | 35 | 237 | 272 |
| DimensionMaster | 140 | 696 | 836 |
| LanceMaster | 69 | 286 | 355 |
| Valtan | 52 | 346 | 398 |
| Warlord | 149 | 652 | 801 |
| 전체 | 445 | 2,217 | 2,662 |

현재 `CEffect_Tool::Refresh_ResourceCatalog`는 2,662개 파일을 재귀 스캔하지만 첫 폴더인
class/boss 도메인과 그 아래 Mesh/Texture category를 분리하지 않는다. Resource Browser의
Category에는 `DimensionMaster/Textures/FX_TEX_00` 같은 전체 경로가 섞이고 All Effects의
class 선택, Data Files 선택과도 동기화되지 않는다.

이번 목표는 원본 Material slot을 자동 확정하는 것이 아니다. 선택한 저작 도메인의 모든
WModel을 Mesh Shape 후보로, 모든 DDS를 Base/Noise/Mask/Emissive/Dissolve 후보로 빠짐없이
썸네일 표시하고 작업자가 조합·튜닝할 수 있게 한다.

## 유지할 경계

- asset ID는 계속 `Effect/<Domain>/...` Resources-relative 경로를 사용한다.
- `Data/Effects/Authored`의 실제 파일 위치와 Effect asset ID는 바꾸지 않는다.
- Data Files의 class/category는 저작 UI 분류이며 두 번째 저장 정본이 아니다.
- Warlord와 Valtan은 저작 도메인과 Imported 자료를 표시하지만 playable roster에 추가하지 않는다.
- Gunslinger와 Slayer는 실제 Effect 리소스 폴더가 없으므로 다른 class 리소스로 fallback하지 않는다.
- 슬롯 선택은 file kind만 제한한다. Mesh Shape에는 WModel만, 표준 다섯 Material input에는
  선택 도메인의 모든 DDS를 허용한다.
- 기존 thumbnail cache와 `CModel -> CMaterial` 경로를 재사용한다.

## G14. Resource Domain catalog

`EFFECT_RESOURCE_CATALOG_ENTRY`에 `strDomainId`를 추가한다. 첫 경로 component를
`DimensionMaster`, `Warlord`, `Valtan` 같은 stable 저작 도메인으로 사용하고, 그 아래
parent path만 `Meshes/FX_SM_00`, `Textures/FX_TEX_00` 같은 category로 저장한다.

`EFFECT_RESOURCE_DOMAIN_CATALOG`는 도메인별 Model/Texture category와 파일 수를 소유한다.
catalog refresh는 전체 filesystem scan을 local vector/map에 stage하고 정렬·중복 검증을 끝낸
뒤 catalog, domain 목록, category 목록과 revision을 한 번에 commit한다. 실패하면 이전 목록과
thumbnail revision을 유지한다.

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

종료 증거:

- 다섯 도메인의 WModel/DDS 합계가 실제 filesystem 2,662개와 일치한다.
- 평상시 frame에는 filesystem 재순회가 없고 기존 revision cache를 유지한다.

## G15. 클래스 선택과 전체 Mesh/Texture 팔레트

Resource Browser에 `Authoring Category` combo를 추가한다. All Effects에서 Lance Master,
Artist, Dimension Master를 선택하면 실제 폴더가 있는 동일 도메인으로 전환한다. Warlord와
Valtan은 Resource Browser 또는 Data Files category에서 직접 선택한다.

Resource Browser visible index cache key에 domain ID를 추가한다. 선택 도메인, file kind,
하위 category, search가 모두 맞는 entry만 표시한다. 하위 category `All`은 해당 도메인의
동일 file kind 전체를 뜻한다.

다섯 표준 Texture slot은 semantic filename 필터를 강제하지 않는다. 작업자는 선택 도메인의
모든 DDS를 어느 표준 Texture slot에도 넣을 수 있다. 기존 Bind Selected, Clear Slot,
DDS/WModel thumbnail과 active/bound 테두리는 유지한다.

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`

종료 증거:

- DimensionMaster 선택 시 140 WModel/696 DDS가 하위 category 합계로 모두 보인다.
- Artist, LanceMaster, Valtan, Warlord도 실제 폴더 합계와 일치한다.
- 다른 도메인의 asset은 선택 도메인 grid에 섞이지 않는다.

## G16. Data Files 저작 도메인 분류

`EFFECT_DATA_FILE_ENTRY`에 `strDomainId`를 추가한다. 분류 우선순위는 다음과 같다.

1. `Data/Effects/Imported/<Domain>/...`의 첫 상대 경로 component
2. `PlayerSkills.json`에서 Effect ID를 소유한 playable class
3. `effect.<domain>.` stable asset ID token
4. 어느 근거도 없으면 `Uncategorized`

Data Files는 같은 `Authoring Category` combo로 필터한다. 각 도메인 안에서는 Authored,
Imported, Imported Draft source와 asset ID 순으로 정렬한다. Imported Draft 행을 선택하면
재생은 계속 차단하지만 같은 도메인의 Resource Browser가 즉시 열려 receipt를 보며 리소스를
조합할 수 있다.

Save/Save As는 기존 flat `Data/Effects/Authored/<effectId>.effect.json`과 원자 저장을 유지한다.
물리 폴더 이동, authoring schema version 증가, Warlord runtime onboarding은 하지 않는다.

수정 파일:

- `Client/Public/Effect_Tool.h`
- `Client/Private/Effect_Tool.cpp`
- `Tools/ProjectAudit/Test-EffectToolFinal.ps1`
- `.md/GB/08-06/2026-08-06_EFFECT_CLASS_RESOURCE_AUTHORING_RESULT.md`

## 구현·검증 순서

1. header의 resource/data domain 상태와 cache key를 추가한다.
2. resource catalog의 `parse -> validate -> stage -> commit`을 도메인 기준으로 교체한다.
3. Resource Browser의 domain/category combo와 All Effects class 동기화를 연결한다.
4. Data Files의 도메인 분류, 필터, 선택 동기화를 연결한다.
5. Effect final audit에 다섯 물리 폴더와 코드 계약 검사를 추가한다.
6. Client x64 Debug build, Effect final audit, ProjectAudit, 정본 Debug 회귀를 실행한다.
7. F1에서 다섯 도메인 전환, WModel/DDS thumbnail, 다섯 Texture slot binding과 Data Files
   category를 수동 확인한다. 자동 조작하지 못한 항목은 RESULT에서 별도 표시한다.
