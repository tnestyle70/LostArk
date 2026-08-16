# 2026-08-17 Effect Tool 텍스처 종류 필터 구현 계획

branch: `feature/effect-tool-texture-kind-filter`

발탄 패턴 이펙트를 하나씩 저작할 때 Resource Library에서 텍스처를 고르는 비용을 줄인다.
Mesh에만 있는 의미 분류 콤보를 Texture에도 같은 계약으로 추가한다.

## G00. 목표와 종료 증거

```text
목표    Resource Library의 Textures 목록을 파일명 kind 토큰으로 좁힌다.
종료    Effect Tool > Resource Library > Textures에서 Texture Kind 콤보가 보이고
        선택한 종류만 썸네일 그리드에 남는다.
비목표  DDS 포맷/해상도 기반 자동 분류, 썸네일 카드의 포맷 배지,
        Authored 문서나 EffectCatalog 변경.
```

이 변경은 저작 UI의 검색 보조다. 저장 계약인 `assetId`와 slot binding은 그대로다.

## G01. 현재 실측

### G01.1 Valtan 텍스처 코퍼스

`Client/Bin/Resources/Effect/Valtan/Textures` 하위 DDS 346개를 파일명 토큰과 DDS 헤더로 실측했다.

파일명은 `fx_<bucket>_<kind>_<index>[_variant]` 4토큰이다.

```text
fx_i_shockwave_02.dds  ->  fx / i / shockwave / 02
```

두 번째 토큰은 원본 패키지 버킷이며 물리 폴더와 거의 1:1이다. 저작 의미가 없다.

```text
a -> FX_TEX_00,03,04      d -> FX_TEX_02       k,m,o -> FX_TEX_05
i,j,g,f,h -> FX_TEX_04    e -> FX_TEX_03       c     -> FX_TEX_01
```

기존 `Resource Folder` 콤보가 정확히 이 버킷으로 나뉘어 있다. 그래서 폴더를 골라도
링·노이즈·디캘·번개가 섞여 나온다. 실제 분류축은 세 번째 kind 토큰이다.

```text
atypical 70 | noise 37 | decal 26 | fluid 23 | normal 18 | hit 16
cloud 14 | ring 13 | glow 10 | fragment 8 | trail 8 | fire 7
environ 7 | electric 6 | auraline 6 | shockwave 4 | wave 4 | line 4
```

### G01.2 DDS 헤더 실측 — 이번 G의 비목표지만 분류 근거

```text
DXT1  241   알파 없음. Base에 넣으면 가산 전용
DXT5   78   알파 있음. 알파블렌드 Base / Mask / Dissolve 가능
ATI2   25   2채널 BC5 노멀맵. Base 금지
정사각 307 / 비정사각 39 (512x256 트레일 스트립, 128x4 그라디언트 램프 등)
```

ATI2 25개는 전부 이름에 `_n` 또는 `normal`을 가진다. 역은 성립하지 않아
이름만으로 노멀맵을 확정할 수 없다. 포맷이 정본이고 이름은 힌트다.
이번 G는 이름 힌트만 사용하고 포맷 판정은 도입하지 않는다.

### G01.3 기존 Mesh 분류 경로

`Client/Private/Effect_Tool.cpp`에 이미 같은 모양의 계약이 있다.

```text
Matches_MeshShapeCategory()          파일명 부분문자열 매칭 판정
Render_ResourceGrid()                Mesh Shape Category 콤보 렌더
Rebuild_ResourceBrowserView()        strShapeCategory로 view 캐시 키 구성 + 필터 적용
m_strMeshShapeCategory               선택 상태(session state)
```

Mesh 콤보는 `bMeshAuthoringDraft && MODEL == eWanted`로 게이트된다.
Texture는 일반 slot binding 흐름에서도 필요하므로 `TEXTURE == eWanted`만으로 노출한다.

### G01.4 다른 세션과의 경계

동시 진행 중인 Codex 세션의 미커밋 파일은 다음과 같다. 이번 G가 건드리는 파일과 겹치지 않는다.

```text
Client/Private/Level_Bern.cpp / Level_CharacterSelect.cpp / Level_Development.cpp
Client/Private/Level_Lobby.cpp / Level_ValtanArena.cpp / MainApp.cpp
Client/Public/MainApp.h
Client/Bin/DataFiles/World/LV_LUT_HEARTRB_ED.worlddestruction*.json
Framework.slnLaunch
```

## G02. 변경할 파일

```text
Client/Public/Effect_Tool.h     선언 1개 rename, 멤버 1개 rename, 멤버 1개 추가
Client/Private/Effect_Tool.cpp  판정 함수 1개 추가, 콤보 1개 추가, 호출부 2곳 수정
```

새 파일이 없으므로 `.vcxproj` / `.vcxproj.filters` 등록 변경은 없다.

## G03. H 계약 delta

`Rebuild_ResourceBrowserView`의 다섯 번째 인자는 이제 Mesh 형상과 Texture 종류를
모두 운반한다. 이름이 운반하는 값을 설명하도록 바꾼다.

```text
strShapeCategory              -> strKindCategory
m_strResourceViewShapeCategory -> m_strResourceViewKindCategory
```

`m_strResourceViewKindCategory`는 마지막으로 view를 만들 때 사용한 분류를 보존하는
캐시 키다. 선택 상태가 아니라 무효화 판정용이며 owner는 browser view다.

추가 멤버는 하나다.

```text
string m_strTextureKindCategory = "All";
```

Texture 목록의 현재 선택 분류를 소유하는 session state다. Document 데이터가 아니며
저장되지 않는다. `m_strMeshShapeCategory`와 독립이므로 Mesh/Texture를 오가도
각자의 선택이 유지된다. 기본값 `"All"`은 필터 없음을 뜻하고 빈 문자열로 대체하지 않는다.

## G04. CPP delta

### G04.1 `Matches_TextureKindCategory()` 추가

한 줄 책임: assetId 파일명이 선택한 텍스처 종류에 속하는지 판정한다.

`Matches_MeshShapeCategory` 바로 뒤 익명 namespace에 둔다. 호출자는
`Rebuild_ResourceBrowserView` 하나이며 Tool 상태를 읽지 않는 순수 판정이다.

분류와 토큰은 다음과 같다. kind 토큰이 다른 단어 안에 나타나는 경우
(`white`의 `hit`, `slice`의 `ice`, `auraline`의 `line`)는 구분자 `_`로 앵커한다.

```text
Base / Sprite         atypical glow shine star _hit spatter fragment stoneparts aura
Noise / Distortion    noise flow turbulence
Normal / Bump         normal _n. _n_
Decal / Ground        decal grid symbol sector
Ring / Shockwave      ring wave
Trail / Beam          trail _line auraline thunder electric electile
Cloud / Smoke / Fire  cloud smoke fire fogsheet
Fluid / Water         fluid liquid water softriver _ice
Other                 위 토큰 전체의 부정
```

`All`과 빈 문자열은 항상 통과시킨다. 알 수 없는 분류 문자열도 통과시켜
필터가 목록을 조용히 비우지 않게 한다.

한 파일이 두 분류에 동시에 걸릴 수 있다. `fx_a_fluid_011_n`은 Fluid와 Normal
양쪽에서 보인다. 검색 보조이므로 중복 노출을 허용한다.

### G04.2 `Render_ResourceGrid()` — Texture Kind 콤보

Mesh Shape Category 블록 바로 뒤, `bCompatibleSlot` 재계산 직전에 삽입한다.
`TEXTURE == eWanted`일 때만 렌더하므로 Mesh 목록에는 나타나지 않는다.

선택이 바뀌면 `m_strTextureKindCategory`를 교체하고 `m_iResourceViewRevision`을
`UINT64_MAX`로 만들어 다음 프레임 view 재구성을 강제한다. Mesh 콤보와 같은 방식이다.

### G04.3 `Render_ResourceGrid()` — 호출 인자

```text
기존  bMeshAuthoringDraft ? m_strMeshShapeCategory : "All"
변경  TEXTURE == eWanted ? m_strTextureKindCategory
                         : (bMeshAuthoringDraft ? m_strMeshShapeCategory : "All")
```

Mesh 경로의 기존 동작은 그대로다.

### G04.4 `Rebuild_ResourceBrowserView()` — 필터 분기

기존 MODEL 분기 뒤에 TEXTURE 분기를 추가한다. 두 분기는 배타적이며
같은 `strKindCategory` 인자를 각자의 판정 함수로 넘긴다.

view 캐시 비교와 저장에 쓰던 `m_strResourceViewShapeCategory`는
`m_strResourceViewKindCategory`로 이름만 바뀐다. 무효화 규칙은 그대로다.

## G05. 호출 흐름

```text
사용자가 Texture Kind 콤보에서 항목 선택
  -> m_strTextureKindCategory 교체
  -> m_iResourceViewRevision = UINT64_MAX
  -> 다음 프레임 Render_ResourceGrid()
    -> Rebuild_ResourceBrowserView(TEXTURE, Filter, Domain, Category, KindCategory)
      -> 캐시 키 불일치 확인
      -> m_ResourceCatalog 순회하며 Staged에 인덱스 수집
        -> Matches_TextureKindCategory() 판정
      -> m_VisibleResourceIndices = move(Staged)   commit
      -> 캐시 키 5개 갱신
    -> ImGuiListClipper로 썸네일 그리드 렌더
```

실패 경로가 없다. 판정 함수는 예외를 던지지 않고 파일 I/O도 하지 않는다.
분류가 아무것도 매치하지 않으면 그리드가 비고 `Resource Folder`와 검색어를
그대로 두므로 이전 선택 slot과 binding은 보존된다.

## G06. 검증

```text
build     MSBuild Client.vcxproj /p:Configuration=Debug /p:Platform=x64
          작업 디렉터리는 Client/Default
분류 검증  346개 파일명에 토큰 규칙을 적용해 분류별 개수와 Other 잔여를 실측
git       git diff --check
runtime   F1 > Effect Tool > Resource Library > Textures에서 Texture Kind 전환
          (사용자 육안 확인 항목)
```

Data 변경이 없으므로 Effect publisher validation 대상이 아니다.
