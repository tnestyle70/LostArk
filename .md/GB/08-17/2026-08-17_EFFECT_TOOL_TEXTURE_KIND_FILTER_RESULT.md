# 2026-08-17 Effect Tool 텍스처 종류 필터 결과

branch: `feature/effect-tool-texture-kind-filter`
기준 계획: `.md/GB/08-17/2026-08-17_EFFECT_TOOL_TEXTURE_KIND_FILTER_IMPLEMENTATION_PLAN.md`

## 1. 실제 구현 상태

계획한 G01~G05를 전부 반영했다. 변경은 두 파일이다.

```text
Client/Private/Effect_Tool.cpp   +90 -6
Client/Public/Effect_Tool.h      +4  -2
```

### 1.1 `Client/Public/Effect_Tool.h`

```text
L317  Rebuild_ResourceBrowserView 인자
      const std::string& strShapeCategory -> const std::string& strKindCategory
L720  string m_strResourceViewShapeCategory -> string m_strResourceViewKindCategory
L722  string m_strTextureKindCategory = "All";   추가
```

`m_strMeshShapeCategory`는 그대로 두었다. Mesh 형상 선택과 Texture 종류 선택은
서로 독립 session state이며 Mesh/Texture 라디오를 오가도 각자 유지된다.

### 1.2 `Client/Private/Effect_Tool.cpp`

```text
Matches_MeshShapeCategory 직후   Matches_TextureKindCategory() 추가 (익명 namespace)
Render_ResourceGrid              Mesh Shape Category 블록 뒤에 Texture Kind 콤보 추가
                                 TEXTURE == eWanted 일 때만 렌더
Render_ResourceGrid              Rebuild_ResourceBrowserView 다섯 번째 인자를 kind-aware로 교체
Rebuild_ResourceBrowserView      인자/캐시 키 rename, TEXTURE 필터 분기 추가
```

Mesh 경로의 기존 동작은 바뀌지 않았다. `Rebuild_ResourceBrowserView`의 MODEL 분기와
view 무효화 규칙은 rename 외 변경이 없다.

새 파일이 없어 `.vcxproj` / `.vcxproj.filters` 등록 변경은 없다.

## 2. 자동 검증 — 실행함

### 2.1 Client x64 Debug 빌드

```text
MSBuild Client.vcxproj /p:Configuration=Debug /p:Platform=x64 /m
작업 디렉터리 Client/Default

경고 21개  오류 0개   경과 00:00:15
```

경고 21개는 전부 기존 것이다. C4819 13건은 Codex 세션이 수정 중인 `MainApp.cpp`와
CP949 Shared 헤더, LNK4099 8건은 `DirectXTKd.lib`의 PDB 부재다.
이번 변경 파일 두 개는 ASCII이며 새 경고를 만들지 않았다.

산출물 최신화를 timestamp로 확인했다.

```text
Client/Default/Client/x64/Debug/Effect_Tool.obj   2026-08-17 01:22:34
Client/Bin/Debug/Client.exe                       2026-08-17 01:22:37
```

`Client/Default/x64/Debug/`의 동명 obj는 이전 출력 구조가 남긴 stale 파일이며
현재 빌드가 사용하는 중간 디렉터리가 아니다.

### 2.2 분류 규칙 코퍼스 검증

구현한 토큰 규칙을 `Client/Bin/Resources/Effect/Valtan/Textures`의 DDS 346개에
그대로 적용해 분류 분포를 실측했다.

```text
Base / Sprite          109
Noise / Distortion      43
Normal / Bump           37
Decal / Ground          34
Fluid / Water           29
Trail / Beam            27
Cloud / Smoke / Fire    25
Ring / Shockwave        22
Other                   34
```

합이 346을 넘는 것은 의도된 중복이다. `fx_a_fluid_011_n`처럼 Fluid와 Normal에
동시에 걸리는 파일이 있으며 검색 보조이므로 허용한다.

`Normal / Bump` 규칙(`normal` / `_n.` / `_n_`)은 DDS 헤더가 ATI2인 25개를
누락 없이 전부 포함한다. 추가로 걸리는 12개는 이름이 `normal` 계열이지만
DXT1로 압축된 파일들이라 의미상 오분류가 아니다.

`Other` 34개의 내역은 다음과 같다. fx 스프라이트가 아닌 것이 대부분이다.

```text
EFMASTER_MATERIAL_PROLOGUE 계열 8   brdf_beckmann_spec, diffuse, flat_black,
                                    flat_red, hdr07_1, spec, statefx_default, normal
MN_ELID_00 / MN_RPBF_01 계열 11     캐릭터형 d/s/e/m/cm 세트
environ / environment        10     분류 미할당
그 외                         5     blankwhite, rectangle, rainbowmosaic,
                                    risingcolor, risingforce, turtlespec
```

### 2.3 잔여 참조와 diff

```text
git grep strShapeCategory / m_strResourceViewShapeCategory   0건
git diff --check                                             clean
```

## 3. 수동 검증 — 미실행

다음은 사용자가 실제 Client에서 직접 확인할 항목이다. 에이전트는 판정하지 않았다.

```text
F1 -> Effect Tool -> Resource Library
  Textures 라디오 선택 시 Texture Kind 콤보가 보이는지
  Meshes 라디오에서는 보이지 않고 Mesh Shape Category만 남는지
  Authoring Category = Valtan에서 종류 전환 시 썸네일 그리드가 실제로 좁혀지는지
  종류 선택 상태에서 slot 카드에 바인딩이 정상 동작하는지
  Meshes <-> Textures를 오갈 때 각자의 분류 선택이 보존되는지
```

## 4. 남은 경계

```text
DDS 포맷 기반 분류 없음
  DXT1 241 / DXT5 78 / ATI2 25는 실측했지만 UI에 노출하지 않는다.
  m_ResourceCatalog 엔트리가 포맷을 들고 있지 않아 카탈로그 스캔 확장이 필요하다.
  Base 슬롯에 ATI2를 바인딩해도 지금은 경고가 없다.

해상도/종횡비 힌트 없음
  512x256 트레일 스트립, 128x4 그라디언트 램프를 구분하지 않는다.

Valtan 외 도메인 미검증
  분류 규칙은 fx_<bucket>_<kind> 명명을 가정한다. 다른 class 폴더의
  명명 편차는 확인하지 않았다.

environ / environment 10개 미분류
  Other로 남아 있다. 별도 분류가 필요한지는 실제 저작에서 판단한다.
```

## 5. 발탄 이펙트 저작 시 참고 실측

이번 조사에서 확정한 사실 중 이후 복원 작업에 계속 쓰이는 것만 남긴다.

```text
파일명 규칙   fx_<bucket>_<kind>_<index>[_variant]
              bucket(두 번째 토큰)은 원본 패키지 버킷이며 저작 의미가 없다.
              기존 Resource Folder 콤보가 이 버킷 축이라 도움이 되지 않는다.
              kind(세 번째 토큰)가 실제 분류축이다.

변형 접미사   _1 _2 는 변형, _n 은 노멀, _cl / _ycl 은 같은 base의 색 변형 페어다.
              _cl / _ycl 의 정확한 의미는 원본 명세가 없어 미해독이며
              분류축이 아니라 정렬축으로 다룬다. 해당 파일은 49개다.

DDS 포맷      DXT1 알파 없음(가산 전용) / DXT5 알파 있음 / ATI2 노멀맵(Base 금지)
              ATI2 25개는 이름에 _n 또는 normal 을 반드시 가진다. 역은 성립하지 않는다.

Valtan 코퍼스 Textures 346 DDS / Meshes 52 wmodel
              Textures 하위 폴더 15개는 전부 원본 패키지 버킷이다.
```

## 6. 별개 사실 — All Effects의 Valtan 항목

이번 작업 중 확인했으며 이 변경의 범위는 아니다.

`All Effects`에서 Valtan을 선택하면 `Refresh_ValtanBossPatternEffects()`가
`Data/Animation/Authored/Valtan/Valtan.patterneffects.json`을 스테이징한다.
현재 이 문서의 binding은 1건이다.

```text
VALTAN_WHIRLWIND | valtan.attack.whirlwind.active | effect.valtan.pattern.420633.active
```

즉 Resource Library의 재료 346개와 달리 완성 이펙트 문서는 31 패턴 중 1개다.
`.md/GB/08-15/2026-08-15_VALTAN_PATTERN_EFFECT_RESTORATION_SURVEY_AND_PLAN.md`가
분석한 actionId 어댑터 부재와 같은 사실이다.
