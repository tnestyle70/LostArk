# 2026-08-04 renamed map material path recook PLAN

## 0. C1~C8 관점

- **C1 기준계(핵심)**: WModel의 `Resource/Map/<namespace>/...`와 실제 `Client/Bin/Resources/Map/<namespace>/...`를 동일 namespace로 맞춘다.
- **C2 이동>계산(핵심)**: 런타임에서 파일명을 추측하지 않고 오프라인 recook으로 canonical texture path를 저장한다.
- C3 공유는 비싸다: 모델별 texture 폴더를 유지해 전역 파일명 충돌을 만들지 않는다.
- C4 수명은 선언된다: Resources payload만 교체하고 Prototype/Clone 수명은 바꾸지 않는다.
- C5 이산화와 오차: geometry와 `--scale 100`은 유지하고 material section만 같은 입력으로 다시 생성한다.
- C6 가지치기: Character Select 55개와 Training 302개를 stage한 뒤 357/357일 때만 runtime WModel을 교체한다.
- **C7 권위와 정합성(핵심)**: MapCatalog의 `runtimeAssetRoot`와 WModel 내부 material path를 같은 저장 계약으로 만든다.
- C8 검증이 병목: 918개 texture reference의 실제 파일 존재, Debug/Release build, runtime entry를 검증한다.

## 1. 문제 해결 ①~⑤

① 문제·제약: `CHARACTERSELECTMAP` 첫 WModel의 material이 옛 `LV_LOBBY_CLASSSELECT_SL00` 경로를 읽어 `CMaterial` 생성이 실패하고 Loader가 모달 창에서 멈춘다.
② 단순 해법의 문제: 옛 Area 폴더를 복제하거나 파일명만으로 찾으면 직관적 namespace 계약을 깨고 잘못된 동명 texture를 조용히 바인딩할 수 있다.
③ 해결 방식: 기존 extracted glTF/material manifest를 입력으로 두 맵 357개를 실제 namespace로 전부 stage-recook하고, 모든 WModel/texture reference 검증 후 runtime WModel만 교체한다.
④ 비교: geometry와 placement를 다시 추출하지 않는다. `CModel -> CMaterial` 경로와 per-asset texture 폴더는 유지한다.
⑤ 대가: 357개 binary payload가 로컬에서 다시 생성된다. Resources는 Git 대상이 아니며 폐기한 immutable pack `.4`는 재생성하지 않는다.

## 2. 자료구조·알고리즘 핵심

- 정의 데이터: `Data/Maps/MapCatalog.json`의 `runtimeAssetRoot`가 현재 runtime namespace 정본이다.
- 오프라인 입력: `Resource_LostArk/01_Extracted/Map/LevelAssets_Textured/<Area>/<Asset>/manifest.json`과 source glTF/bin/texture다.
- 저장 불변식: WModel의 모든 `Resource/Map/<namespace>/...`는 flat Resources root 아래 실제 파일로 resolve되어야 한다.
- 생성 알고리즘: manifest parse → converter argument stage → temporary WModel 생성 → WINT/WMOD/info/texture path 검증 → 357개 전부 성공 시 runtime WModel 교체다.
- 실패 알고리즘: 한 asset이라도 cook 또는 validation에 실패하면 runtime WModel을 교체하지 않고 기존 payload를 유지한다.

## 3. 파일 목록

| 구분 | 절대 경로 | 역할 |
|---|---|---|
| 수정 | `C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp` | texture load 실패 경로를 debug log로 남기고 Loader worker 모달을 제거 |
| 수정 | `C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1` | extracted map WModel 내부 texture reference의 canonical resolve 검사 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/07-29/gotchas.md` | runtime namespace rename 뒤 recook 필수 조건 기록 |
| 수정 | `C:/Users/user/Desktop/LostArk/.md/GB/08-03/2026-08-03_LOSTARK_CIRCULAR_MAP_REEXTRACTION_RESULT.md` | 기존 material 검증 주장의 namespace 누락을 교정 |
| 생성 | `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Map/CHARACTERSELECTMAP/**/*.wmodel` | 실제 `CHARACTERSELECTMAP` texture path로 recook된 로컬 payload |
| 생성 | `C:/Users/user/Desktop/LostArk/Client/Bin/Resources/Map/TRAININGMAP/**/*.wmodel` | 실제 `TRAININGMAP` texture path로 recook된 로컬 payload |

## 4. 파일별 전체 구현 코드

### 4-1. C:/Users/user/Desktop/LostArk/Engine/Private/Material.cpp

변경 종류: 함수 교체
적용 위치: anonymous namespace `AddTexture`

```cpp
HRESULT AddTexture(ComPtr<ID3D11Device> pDevice,
	const filesystem::path& path,
	aiTextureType type,
	vector<ComPtr<ID3D11ShaderResourceView>> (&textures)[AI_TEXTURE_TYPE_MAX])
{
	if (path.empty())
		return S_OK;
	ComPtr<ID3D11ShaderResourceView> resource;
	const HRESULT result = LoadTexture(
		pDevice, path, IsColorTextureSlot(type), resource);
	if (FAILED(result))
	{
		wstring message = L"[CMaterial] Texture load failed: ";
		message += path.wstring();
		message += L"\n";
		OutputDebugStringW(message.c_str());
		return result;
	}
	textures[type].push_back(resource);
	return S_OK;
}
```

변경 종류: 함수 교체
적용 위치: 두 `CMaterial::Create` overload

```cpp
shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice, ComPtr<ID3D11DeviceContext> pContext, const aiMaterial* pAIMaterial, const char_t* pModelFilePath)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));

	if (FAILED(pInstance->Initialize(pAIMaterial, pModelFilePath)))
	{
		OutputDebugStringA("[CMaterial] Assimp material initialization failed.\n");
		return nullptr;
	}

	return pInstance;
}

shared_ptr<CMaterial> CMaterial::Create(ComPtr<ID3D11Device> pDevice,
	ComPtr<ID3D11DeviceContext> pContext, const MODEL_MATERIAL_DATA& material)
{
	auto pInstance = shared_ptr<CMaterial>(new CMaterial(pDevice, pContext));
	if (FAILED(pInstance->Initialize(material)))
	{
		OutputDebugStringA("[CMaterial] Binary material initialization failed.\n");
		return nullptr;
	}
	return pInstance;
}
```

### 4-2. C:/Users/user/Desktop/LostArk/Tools/ProjectAudit/Invoke-ProjectAudit.ps1

변경 종류: 함수 추가
적용 위치: `Get-ProjectItems` 다음

```powershell
function Get-WModelTextureReferences {
    param([string]$Path)

    $decoded = [Text.Encoding]::Unicode.GetString(
        [IO.File]::ReadAllBytes($Path))
    return @([regex]::Matches(
        $decoded,
        '(?:Resource|Resources)/[^\x00]{1,259}?\.(?:dds|png|tga|jpg|jpeg|bmp)',
        [Text.RegularExpressions.RegexOptions]::IgnoreCase) |
        ForEach-Object Value |
        Sort-Object -Unique)
}
```

변경 종류: 블록 교체
적용 위치: `maps.extracted-area-runtime-roots`의 runtime model 존재 검사 loop

```powershell
		if (Test-Path -LiteralPath $runtimeDirectory -PathType Container) {
			foreach ($modelPath in $modelPaths) {
				$modelFile = Join-Path 'Client\Bin\Resources' $modelPath.Replace('/', '\')
				if (-not (Test-Path -LiteralPath $modelFile -PathType Leaf)) {
					$singleAreaContractErrors.Add("$($area.id): missing runtime model")
					continue
				}

				foreach ($textureReference in @(Get-WModelTextureReferences $modelFile)) {
					$normalizedReference = $textureReference.Replace('\', '/')
					$relativeTexture = $normalizedReference -replace
						'^Resources?/(?:LostArk/)?', ''
					if ($relativeTexture -eq $normalizedReference -or
						[IO.Path]::IsPathRooted($relativeTexture)) {
						$singleAreaContractErrors.Add(
							"$($area.id): invalid material texture path")
						continue
					}
					$textureFile = Join-Path 'Client\Bin\Resources' `
						$relativeTexture.Replace('/', '\')
					if (-not (Test-Path -LiteralPath $textureFile -PathType Leaf)) {
						$singleAreaContractErrors.Add(
							"$($area.id): unresolved material texture")
					}
				}
			}
			$runtimeManifestPath = Join-Path $runtimeDirectory 'map_asset_runtime_manifest.json'
			if (Test-Path -LiteralPath $runtimeManifestPath -PathType Leaf) {
				$runtimeManifest = Read-Json $runtimeManifestPath
				$runtimeManifestKeys = @($runtimeManifest.assets | ForEach-Object {
					"$($_.assetId)|$(([string]$_.model).Replace('\', '/'))"
				} | Sort-Object)
				$catalogKeys = @($catalogAssetKeys | Sort-Object)
				if ($runtimeManifest.areaId -ne $area.id -or
					[int]$runtimeManifest.assetCount -ne $modelPaths.Count -or
					@($runtimeManifest.assets).Count -ne $modelPaths.Count -or
					($runtimeManifestKeys -join "`n") -ne ($catalogKeys -join "`n")) {
					$singleAreaContractErrors.Add("$($area.id): runtime manifest set/count")
				}
			} else {
				$singleAreaContractErrors.Add("$($area.id): missing runtime manifest")
			}
		} else {
			$singleAreaContractErrors.Add("$($area.id): missing runtime directory")
		}
```

변경 종류: 블록 교체
적용 위치: `$loaderFactoryFiles`

```powershell
	$loaderFactoryFiles = @(
		'Engine\Private\Shader.cpp',
		'Engine\Private\Material.cpp',
		'Engine\Private\Model.cpp',
		'Engine\Private\Navigation.cpp',
		'Client\Private\Camera_Free.cpp',
		'Client\Private\Character.cpp',
		'Client\Private\Part_Body.cpp',
		'Client\Private\Part_Equipment.cpp',
		'Client\Private\Valtan.cpp',
		'Client\Private\Body_Valtan.cpp')
```

## 5. Runtime payload recook 절차

한 번에 runtime을 덮어쓰지 않는다. 두 외부 source Area를 각 runtime namespace로 임시 root에 recook하고 다음을 전부 검증한 뒤 해당 asset ID의 `.wmodel`만 `Client/Bin/Resources/Map/<namespace>`에 교체한다.

```text
LV_LOBBY_CLASSSELECT_SL00 -> CHARACTERSELECTMAP -> 55 models
LV_SHS_RCARENA_D          -> TRAININGMAP        -> 302 models
```

각 converter 명령의 고정 인자는 다음과 같다.

```text
<source.gltf> -o <staged.wmodel>
--pretransform
--no-auto-textures
--scale 100
--material-remap/--normal-remap/... <material>=Resource/Map/<runtime namespace>/<asset ID>/<manifest source path>
```

material에 diffuse가 없으면 다음 명시적 fallback을 저장한다.

```text
Resource/Map/<runtime namespace>/_shared/map_fallback_gray.png
```

## 6. 프로젝트 등록

새 C++/data source 파일은 없다. `Material.cpp`와 ProjectAudit 기존 항목만 수정하므로 `.vcxproj`와 `.vcxproj.filters` 변경은 없다. Resources WModel은 외부 payload이며 Git project item으로 등록하지 않는다.

## 7. 적용 순서와 검증

1. 현재 55/167, 302/751의 unresolved material path를 baseline으로 보존한다.
2. 357개 WModel을 별도 staging root에 recook한다.
3. WINT/WMOD, model count, 918개 texture reference, actual texture file을 검증한다.
4. 전부 PASS일 때 runtime WModel을 교체한다.
5. Engine Debug/Release → UpdateLib → Client Debug/Release를 빌드한다.
6. `Invoke-ProjectAudit.ps1 -DeepAssetHash`와 `git diff --check`를 실행한다. `.3` inventory 차이는 폐기된 `.4` 경계로 별도 기록한다.
7. `Client/Default`에서 Server + Client를 실행해 Lobby → Character Select 진입과 `CMaterial` 모달 부재, 원형 홀 표시를 확인한다.
8. Test 진입에서 `TRAININGMAP` 재질 로드를 확인한다.
