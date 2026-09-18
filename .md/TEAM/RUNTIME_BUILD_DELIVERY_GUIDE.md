# Runtime 빌드 ZIP·Drive Resources 전달 가이드

## 경계

팀 Git은 코드와 `Data` 저작 정본을 관리하고, 팀장 Drive는
`Client/Bin/Resources` 물리 리소스를 관리한다. 실행 배포본은 이미 빌드·publish된 EXE, DLL,
CSO와 필요한 `Client/Server Bin/DataFiles`를 전달한다. 제품이 직접 읽는 `Data` JSON도 실행 위치에서
같은 버전으로 준비해야 한다. Resources를 배포 ZIP, manifest 또는 Git 정본으로 승격하지 않는다.

## Portable 실행 배포본

v5 portable은 압축 해제 폴더 안의 Client/Server와 Data/DataFiles를 사용한다.
기존 LostArk 폴더를 선택하는 이유는 `Client/Bin/Resources`를 읽기 위해서다.
선택한 저장소에 EXE·DLL·Data를 설치하거나 덮어쓰지 않으며, 그 저장소의 실행 파일도 사용하지 않는다.
Resources 폴더 자체를 선택할 수도 있다. 받는 PC의 portable 폴더에는 `Framework.sln`이나 Git checkout이 필요하지 않다.

현재 v5는 최종 Release 빌드·ZIP 생성·파일 검증을 완료했다. 파일은
`LostArk-Release-20260919-v5-192.168.0.14.zip`이며, 크기·SHA256·검증 근거와 실제 실행 명령은
[Release ZIP 안내](../../Release/zipRelease.md)에서 관리한다. 제품 빌드·ZIP 검증과 사용자의 실제 4인 화면 검증을 구분한다.

| 포함 경로 | 소비 계약 |
|---|---|
| `Client/Bin/Release` | Client.exe, Engine.dll, 의존 DLL과 컴파일된 `.cso` |
| `Server/Bin/Release` | Server.exe |
| `Client/Bin/DataFiles`, `Server/Bin/DataFiles` | 같은 게시본의 런타임 데이터와 모든 참조 파일 |
| `Data` | 직접 소비 catalog·pattern·Effect·Sound·UI 등의 JSON, 클래스 `.animevents`와 참조 문서 |
| `Client/Default`, `Server/Default` | 각 프로세스 작업 폴더. 빈 폴더도 보존 |
| root 실행 도구 | `LostArk.exe`, `ServerHost.cmd`, 진단 수집기, README, `bundle-manifest.json` |

모든 Resources와 PNG를 제외한다. `Key_G.png`도 예외가 아니다. `ChangedData`, 중첩 Runtime ZIP,
기존 저장소를 변경하는 설치기는 포함하지 않는다. 직접 소비 JSON은 정상 `Data/...` 경로에 둔다.
HLSL 원본 대신 제품 runtime이 읽는 `.cso`를 실행 파일 옆에 전달한다.

기존 저장소 설치형의 변경분 manifest만으로 portable의 직접 Data 목록을 만들지 않는다.
`CProjectDataRoot`가 bundle의 Data를 가리키면 외부 저장소의 누락 문서를 대신 읽지 않는다.
Effect V1 catalog 참조 외에도 `Effects/V2/Authored`, `Groups`, `Bindings`, 제품 UI·카메라·조명,
클래스 skillbindings·animevents 등 실제 소비 경로와 JSON 하위 참조를 함께 검사한다.
전체 Data를 무조건 복사하지 않고 소비 경로별 목록을 사용하며, authoring/reference는 제품이 직접
읽거나 포함된 descriptor가 참조하는 문서만 전달한다.

`LostArk.exe`는 bundle의 Client를 `Client/Default`에서 시작하고 자식 프로세스에
`LOSTARK_PROJECT_DATA_ROOT=<bundle>/Data`, `LOSTARK_RESOURCE_ROOT=<선택한 Resources>`,
`LOSTARK_SERVER_HOST=192.168.0.14`를 지정한다. 시스템 환경 변수는 영구 변경하지 않는다.
외부 Resources에는 `Fonts, Character, Deploy, Effect, Map, Sound, UI`가 있어야 한다.
manifest의 크기·hash 및 이 폴더들의 존재 검사와 전체 미디어 내용·화면 검증은 구분한다.

서버 PC의 `ServerHost.cmd`는 bundle의 Server를 `Server/Default`에서
`--bind-address 0.0.0.0`으로 실행하고 bundle의 Data를 사용한다.
새 배포본 적용 시 기존 Server를 종료하고 같은 버전의 Server로 시작한다. 다른 PC는 Client만 실행한다.
Client를 시작하지 않는 `LostArk.exe --check <외부 폴더> <receipt.json>`은
실제 EXE·작업 폴더·Data·Resources 경로와 검증 결과를 기록한다.

아래의 `New-LostArkRuntimeDelivery.ps1` / `Install-LostArkRuntimeDelivery.ps1`은
기존 저장소에 runtime을 설치하는 별도 방식이다. portable v5에 이 설치 절차를 적용하지 않는다.

## PR #264~#266에서 최신 main으로 처음 갱신하는 PC

### 적용 대상과 오류 원인

[PR #264](https://github.com/tnestyle70/LostArk/pull/264)의 merge commit `68cabd25`부터
PR #266의 merge commit `9e24aaee` 사이를 사용 중인 PC가 PR #267 이후 `main`을 처음 pull할 때
한 번 적용한다.

PR #267의 commit `53f6d91f`는 기존 Git LFS 추적 `Client/Bin/Resources`를 팀장 Drive 전용
물리 입력으로 전환했다. 이 변경을 만든 PC에는 ignore된 실물 파일이 남을 수 있지만, 이전 commit에서
해당 삭제를 pull하는 다른 PC에서는 Git이 기존 LFS 실물을 작업 폴더에서 제거한다. 반면
`Data/Effects`의 Resources-relative ID는 올바른 제품 계약이므로 그대로 남는다.

그 상태에서 Client를 실행하면 다음처럼 첫 누락 dependency에서 catalog 전체가 fail-close한다.

```text
Effect Catalog Load Failed
Effect source catalog rejected: screen-overlay source resource rejected:
Effect/DimensionMaster/Textures/FX_TEX_02/fx_d_fragment_005.dds;
source file is missing, empty, or exceeds its size limit
```

이 DDS는 첫 번째로 검출된 누락일 뿐이다. 한 파일만 복구하지 말고 현재 팀장 Drive의
`Client/Bin/Resources` 전체를 같은 상대 경로에 준비한다. PR #267 이후 Git index에는 이 경로가
없으므로 현재 commit에서 `git lfs pull`만 다시 실행해도 복구되지 않는다.

### 아직 PR #267 이후 main을 pull하지 않은 PC

Client, Server와 Visual Studio를 닫고 저장소 root에서 `git status --short`를 먼저 확인한다.
출력이 있으면 자기 변경을 안전하게 보존하기 전에는 아래 pull을 진행하지 않으며 `reset`, `clean`,
파일 전체 `ours/theirs`로 정리하지 않는다.

먼저 현재 commit의 LFS 실물을 받은 뒤 `Resources` 전체를 저장소 밖에 복사한다. 팀장 Drive에 이미
같은 전체 폴더가 있으면 그 Drive 폴더가 외부 보존본이다. 별도 보존본이 필요하면 다음처럼 만든다.

```powershell
$repositoryRoot = (git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repositoryRoot)) {
  throw 'Run this command inside the LostArk repository.'
}
$currentBranch = (git branch --show-current).Trim()
if ($LASTEXITCODE -ne 0) {
  throw 'Failed to read the current branch.'
}
if ($currentBranch -ne 'main') {
  throw "Switch to main before this one-time migration. Current branch: $currentBranch"
}
$workingChanges = @(git status --short)
if ($LASTEXITCODE -ne 0) {
  throw 'Failed to inspect the working tree.'
}
if ($workingChanges.Count -ne 0) {
  $workingChanges
  throw 'Preserve all working changes before this one-time migration.'
}

git lfs pull
if ($LASTEXITCODE -ne 0) {
  throw 'git lfs pull failed before the Resources backup.'
}

$localResources = Join-Path $repositoryRoot 'Client\Bin\Resources'
$backupResources = Join-Path (Split-Path $repositoryRoot -Parent) `
  ('LostArk-Resources-before-PR267-' + (Get-Date -Format 'yyyyMMdd-HHmmss'))

robocopy $localResources $backupResources /E /COPY:DAT /DCOPY:DAT /R:2 /W:1
$copyExitCode = $LASTEXITCODE
if ($copyExitCode -ge 8) {
  throw "Resources backup failed with robocopy exit code $copyExitCode"
}

$backupCanary = Join-Path $backupResources `
  'Effect\DimensionMaster\Textures\FX_TEX_02\fx_d_fragment_005.dds'
$backupCanaryItem = Get-Item -LiteralPath $backupCanary -ErrorAction Stop
if ($backupCanaryItem.Length -ne 8320) {
  throw "Resources backup has an invalid Effect canary: $backupCanary"
}
```

마지막 canary 검증이 실패하면 pull하지 말고 팀장 Drive의 현재 Resources를 먼저 받는다. 보존을
확인한 뒤에만 `main`을 갱신한다.

이 백업은 pull이 지우는 이전 실물을 보존하는 안전 사본이지 최신 Product closure 정본이 아니다.
PR #267 기준 다음 세 Effect dependency는 PR #264~#266 Git LFS tree에 없고 팀장 Drive에서만 전달된다.

```text
Effect/Artist/Textures/fx_m_smokesq_01.dds
Effect/DimensionMaster/Textures/BG_OCN_ETC_J/bg_ocn_etc_magicsquare08a_d_kmk.dds
Effect/Esther/Wei/Textures/FX_TEX_00/fx_a_fire_023.dds
```

따라서 canary가 정상인 이전 백업만 복원하고 Client를 실행하지 않는다. pull 뒤 최신 Drive 전체를
반드시 다시 적용하고 Effect source validator로 현재 closure를 확인한다.

```powershell
git fetch --prune
if ($LASTEXITCODE -ne 0) {
  throw 'git fetch failed; main was not updated.'
}
git pull --ff-only origin main
if ($LASTEXITCODE -ne 0) {
  throw 'git pull failed; do not continue to the restore step.'
}
git lfs pull
if ($LASTEXITCODE -ne 0) {
  throw 'git lfs pull failed after updating main.'
}
```

마지막 `git lfs pull`은 현재 Git/LFS dependency를 맞추는 단계이며, 이제 Drive 소유인
`Client/Bin/Resources`를 복구하는 단계가 아니다.

### pull 후 최신 Drive 복원 또는 이미 오류가 발생한 PC

외부 보존본은 먼저 복사해도 되지만 그것만으로 끝내지 않는다. 팀장 Drive의 최신 `Resources` 전체를
source로 지정해 저장소의 `Client/Bin/Resources`에 다시 적용한다. source는 반드시 저장소 밖의
폴더여야 한다. `/E`만 사용하며 대상에만 있는 다른 팀 리소스를 삭제하는 `/MIR`는 사용하지 않는다.

```powershell
$repositoryRoot = (git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repositoryRoot)) {
  throw 'Run this command inside the LostArk repository.'
}
$sourceResources = (Resolve-Path -LiteralPath `
  (Read-Host '팀장 Drive 최신 Resources 폴더의 절대 경로')).Path
$trimChars = [char[]]@(
  [IO.Path]::DirectorySeparatorChar,
  [IO.Path]::AltDirectorySeparatorChar
)
$repositoryRoot = [IO.Path]::GetFullPath($repositoryRoot).TrimEnd($trimChars)
$sourceResources = [IO.Path]::GetFullPath($sourceResources).TrimEnd($trimChars)
$repositoryPrefix = $repositoryRoot + [IO.Path]::DirectorySeparatorChar
$sourcePrefix = $sourceResources + [IO.Path]::DirectorySeparatorChar
if (
  [string]::Equals(
    $sourceResources,
    $repositoryRoot,
    [StringComparison]::OrdinalIgnoreCase
  ) -or
  $sourceResources.StartsWith(
    $repositoryPrefix,
    [StringComparison]::OrdinalIgnoreCase
  ) -or
  $repositoryRoot.StartsWith(
    $sourcePrefix,
    [StringComparison]::OrdinalIgnoreCase
  )
) {
  throw 'Resources source must be outside and must not contain the repository.'
}

$requiredRoots = @('Fonts', 'Character', 'Deploy', 'Effect', 'Map', 'Sound', 'UI')
$missingRoots = @($requiredRoots | Where-Object {
  -not (Test-Path -LiteralPath (Join-Path $sourceResources $_) -PathType Container)
})
if ($missingRoots.Count -ne 0) {
  throw "Drive Resources is missing required roots: $($missingRoots -join ', ')"
}

$localResources = Join-Path $repositoryRoot 'Client\Bin\Resources'

New-Item -ItemType Directory -Path $localResources -Force | Out-Null
robocopy $sourceResources $localResources /E /COPY:DAT /DCOPY:DAT /R:2 /W:1
$copyExitCode = $LASTEXITCODE
if ($copyExitCode -ge 8) {
  throw "Resources restore failed with robocopy exit code $copyExitCode"
}
```

이미 pull한 PC도 같은 Drive 복원 절차를 사용한다. 최신 Drive에 접근할 수 없으면 Client 실행 준비가
끝난 것이 아니므로 전달받을 때까지 멈춘다. 이전 commit의 `Client/Bin/Resources`를 checkout해
새 `main`에 stage하거나 force-add하지 않는다. catalog binding 삭제, validator 완화, 한 DDS만 임시
복사하는 방법도 전체 dependency closure를 복구하지 못한다.

### Client 실행 전 자동 확인

PR #264~#266 Git LFS tree에서 첨부 오류의 canary 실물은 8,320 byte였고 SHA-256은
`193A597BAF328508763B0E6712DC702D604FD1E3B22A311494C26F45470F992C`였다. 이는 이전 파일과
오류 원인을 식별하는 진단값이지, 계속 갱신되는 최신 Drive Resources의 immutable hash 계약이 아니다.

```powershell
$repositoryRoot = (git rev-parse --show-toplevel).Trim()
if ($LASTEXITCODE -ne 0 -or [string]::IsNullOrWhiteSpace($repositoryRoot)) {
  throw 'Run this command inside the LostArk repository.'
}
$localResources = Join-Path $repositoryRoot 'Client\Bin\Resources'
$effectiveResources = $localResources
$effectiveRootSource = 'Client/Bin/Resources'
if (-not [string]::IsNullOrWhiteSpace($env:LOSTARK_RESOURCE_ROOT)) {
  $effectiveResources = (Resolve-Path -LiteralPath `
    $env:LOSTARK_RESOURCE_ROOT -ErrorAction Stop).Path
  $effectiveRootSource = 'LOSTARK_RESOURCE_ROOT'
} elseif (-not [string]::IsNullOrWhiteSpace($env:LOSTARK_SHARED_ASSET_ROOT)) {
  $effectiveResources = (Resolve-Path -LiteralPath `
    $env:LOSTARK_SHARED_ASSET_ROOT -ErrorAction Stop).Path
  $effectiveRootSource = 'LOSTARK_SHARED_ASSET_ROOT'
} else {
  $configurationAdjacentRoots = @(
    (Join-Path $repositoryRoot 'Client\Bin\Debug\Resources')
    (Join-Path $repositoryRoot 'Client\Bin\Release\Resources')
  )
  $presentAdjacentRoots = @($configurationAdjacentRoots | Where-Object {
    Test-Path -LiteralPath $_ -PathType Container
  })
  if ($presentAdjacentRoots.Count -ne 0) {
    $presentAdjacentRoots | ForEach-Object {
      Write-Host "Configuration-adjacent Resources found: $_"
    }
    throw @'
Client prefers a configuration-adjacent Resources folder over Client/Bin/Resources.
Preserve it outside the repository, then move or rename it before this validation.
'@
  }
}
Write-Host "Effective resource root ($effectiveRootSource): $effectiveResources"

$canary = Join-Path $effectiveResources `
  'Effect\DimensionMaster\Textures\FX_TEX_02\fx_d_fragment_005.dds'
$canaryItem = Get-Item -LiteralPath $canary -ErrorAction Stop
if ($canaryItem.Length -le 0) {
  throw "Effect canary is empty: $canary"
}
Write-Host "Effect canary present: $canary ($($canaryItem.Length) bytes)"

python Tools/ResourceDelivery/validate_resource_delivery_policy.py `
  --repository-root $repositoryRoot `
  --require-local
if ($LASTEXITCODE -ne 0) {
  throw 'Resource delivery policy validation failed.'
}

powershell -ExecutionPolicy Bypass `
  -File Tools/EffectPipeline/Validate-EffectSources.ps1 `
  -RepositoryRoot $repositoryRoot `
  -ResourceRoot $effectiveResources
if ($LASTEXITCODE -ne 0) {
  throw 'Effect source validation failed.'
}
```

두 validator가 모두 exit code 0이어야 Client를 실행한다. 첫 명령은 Resources 폴더 존재와 Git 추적
0개 정책만 확인하며, 빈 폴더나 개별 Effect dependency 누락을 검출하지 않는다.
두 번째 명령은 `LOSTARK_RESOURCE_ROOT`, `LOSTARK_SHARED_ASSET_ROOT` 순서의 override와 표준
`Client/Bin/Resources`를 사용해 Product DDS/WModel dependency의 경로, 크기와 내용을 검사한다.
오래된 환경 변수 경로가 있으면 그 root를 표시하고 실패하므로 최신 Resources 전체를 가리키도록
고치거나 불필요한 override를 제거한 뒤 다시 검증한다.

환경 변수 override가 없을 때 Client는 실행 파일 옆의 `Client/Bin/Debug/Resources` 또는
`Client/Bin/Release/Resources`가 존재하면 표준 부모 root보다 먼저 선택한다. 위 명령은 이 숨은
구성별 root를 발견하면 자동 삭제하지 않고 fail-fast한다. 그 내용을 저장소 밖에 보존한 뒤 폴더를
이동하거나 이름을 바꾸고 다시 검증한다. 개인 절대 경로를 코드나 Git 문서에 고정하지 않는다.
PR #267 기준 정상 결과는 `directSourceCount=171`, `resourceFileCount=989`다.

Drive 전용 전환 뒤에는 `Client/Bin/Resources`가 ignore 대상이다. 이 물리 입력을 지우는
`git clean -xfd`를 실행하지 않는다.

## 보내는 PC

Git 동기화, runtime 데이터 게시, 제품 빌드, ZIP 생성은 각각 별도 단계다.
`New-LostArkRuntimeDelivery.ps1`은 이미 있는 파일을 포장하며 publisher나 빌드를 실행하지 않는다.
EXE/DLL/CSO만 전달하고 `DataFiles`를 제외한 ZIP은 전체 실행 배포본으로 사용하지 않는다.

먼저 전달할 commit과 디스크 저장본을 확정하고 아래 두 owner를 순서대로 실행한다.
다른 세션이 같은 정본을 저장 중이면 그 저장이 끝난 최신 revision으로 게시한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Client
if ($LASTEXITCODE -ne 0) { throw 'Client runtime publish failed.' }
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildDomainOwner.ps1 -Owner Server
if ($LASTEXITCODE -ne 0) { throw 'Server runtime publish failed.' }
```

Client owner는 쿠크 projection, 쿠크 맵, Composition, World, Navigation을 게시한다.
Server owner는 World/Navigation, 파괴, Gameplay, 아이템, 탈것, 칭호, 발탄 보상을 준비한다.
`-Owner KoukuSaydon`은 Navigation과 Composition 게시를 포함하지 않으므로 이것만 실행하고
전체 데이터 게시 완료로 판단하지 않는다. 다른 Area의 맵·조명 정본을 바꿨다면 해당 publisher도
별도로 실행한다. Client와 Server의 Navigation은 같은 transaction에서 생성한 파일을 전달한다.

CPP/HLSL 변경이 있으면 같은 저장본에서 정상 증분 Product 빌드를 통과시킨다.
데이터만 다시 게시하는 경우에는 기존 Product 성공 증거와 바이너리 일치를 확인해 재사용할 수 있다.
실행 중인 Client/Server 종료는 EXE 교체·링크 단계에 필요하며 데이터 publish의 선행 조건은 아니다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/Build/Invoke-BuildAndRegression.ps1 -Configuration Release
if ($LASTEXITCODE -ne 0) { throw 'Release Product build failed.' }
```

기존 저장소 설치용 runtime ZIP은 그 뒤 다음처럼 같은 구성으로 만든다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ResourceDelivery/New-LostArkRuntimeDelivery.ps1 `
  -Configuration Debug `
  -RepositoryRoot C:\Users\user\Desktop\LostArk `
  -OutputZip C:\전달\LostArk-Debug-Runtime.zip `
  -IncludePdb
```

ZIP에는 `Client/Bin/Resources`가 0개여야 한다. manifest의 per-file hash는 전송 중 손상과
경로 변조를 설치 전에 거부하기 위한 것이며, Resource pack version/lock 계약이 아니다.

최종 ZIP에서 다음 항목의 존재와 원본 SHA-256 일치를 확인한다.

- 선택한 구성의 Client/Server EXE, 필요한 DLL과 Client CSO.
- `Client/Bin/DataFiles`와 `Server/Bin/DataFiles`의 게시본. Navigation의 region manifest가
  참조하는 지역 grid/policy/blocker까지 포함한다.
- Gameplay bootstrap, World bootstrap, 맵·연출·Composition 등 이번 변경의 실제 소비 파일.

제품은 일부 catalog·패턴·Effect·Sound JSON을 `Data`에서 직접 읽는다.
`New-LostArkRuntimeDelivery.ps1`의 runtime 전용 ZIP에는 `Data`가 없으므로 이 설치 방식을
쓰는 PC는 같은 commit의 `Data`를 준비해야 한다. 과거 v4의 `ChangedData/Data`는 이 설치형의
보충 방식이다. portable v5는 필요한 JSON을 정상 `Data/...` 경로에 포함하고 별도 보충분을 만들지 않는다.
전체 `Data`와 Resources를 무조건 복사하지 않고 실제 직접 소비 경로와 참조 문서 목록을 검증한다.
새 sound/image/model 실물은 기존 Drive 경계로 전달한다.

`EffectCatalog.json`을 포함할 때는 변경된 Effect JSON만 모으지 않는다.
Client 초기화는 catalog의 모든 `DIRECT_AUTHORED_DOCUMENT.authoringPath`가 실제로 존재하는지
확인하므로 그 참조 JSON 전체를 같은 배포본에 포함하고 manifest의 경로·hash로 검증한다.
`screenOverlayPresentationPath`가 있으면 그 Data 문서도 포함한다. 해당 문서가 참조하는
Resources 미디어는 계속 Drive 소유다. 설치형의 같은 commit Data 조건과 두 방식 공통의
물리 Resources 준비 조건은 파일 일부를 보충했다는 이유로 생략하지 않는다.

## 받는 PC: 기존 저장소 설치용 runtime ZIP

1. Git에서 보내는 PC와 같은 commit을 checkout한다.
2. 팀장 Drive의 `Client/Bin/Resources`를 원래 상대 경로에 둔다.
3. 아래 설치 스크립트에 ZIP과 실제 LostArk 물리 폴더를 전달한다.

설치 시에는 편집 내용을 저장한 뒤 해당 폴더의 Client/Server를 종료한다. 수신 ZIP의 안내에서
설치형인지 portable인지 먼저 확인한다. `LostArk.exe`라는 파일명만으로 설치형이라고 판단하지 않는다.
어느 방식도 내부 Client.exe만 따로 복사하지 않는다. Client/Server는 같은 protocol과 게시 데이터를 사용해야 한다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ResourceDelivery/Install-LostArkRuntimeDelivery.ps1 `
  -PackagePath C:\받은파일\LostArk-Debug-Runtime.zip `
  -RepositoryRoot C:\Users\user\Desktop\LostArk
```

설치기는 허용된 `Client/Server Bin` 경로만 받고, 모든 크기/hash를 먼저 검증한 다음 적용한다.
실패하면 이번 실행에서 바꾼 파일을 복구한다. `Client/Bin/Resources`는 읽거나 덮어쓰지 않는다.

로컬 Drive 경계는 다음으로 확인한다.

```powershell
python Tools/ResourceDelivery/validate_resource_delivery_policy.py --require-local
```

## 실행 확인

흰 창이 나타난 뒤 바로 종료되면 설치 성공만으로 Client 시작 성공을 판단하지 않는다.
Release도 `Client/Default/ClientStartup.user.log`에 초기화 단계·HRESULT·상세 오류를,
`ClientExit.user.log`에 종료 사유·PID를 기록한다. 기록은 실행 파일의 위치에서 경로를 찾는다.
로그의 해당 실행 PID와 마지막 실패 단계를 확인하며, 창 색상만으로 Resources·네트워크·
GPU 중 하나를 원인으로 확정하지 않는다. 로그 파일을 쓸 수 없는 경우에도 기존 실패 처리는 유지한다.

시작 감시를 포함한 `LostArk.exe` wrapper는 Client가 10초 이내에 실패 종료하면 오류 창을 표시한다.
기존 설치형 wrapper의 실행기 로그는 `%LOCALAPPDATA%/LostArk/LauncherLogs`에 보존한다.
portable v5 wrapper는 이 로그를 만들지 않으며 오류 창 또는 `--check` receipt와 bundle 안의
Client 로그를 확인한다. 10초 동안 살아 있다는 사실은 Lobby 진입 또는 이후 게임플레이 성공의
증거가 아니다. Client/UI의 최종 화면 확인은 사용자가 수행한다.

Debug 저작 기능은 Server + Client profile에서 사용자가 직접 확인한다. Release에서는 F1/Workbench가
노출되지 않으므로 제품 Lobby/Level 진입만 확인한다. 화면과 음향 fidelity는 자동 설치 결과가 아니라
사용자의 수동 smoke 판정이다.

## 연결 종료 진단 수집

`Tools/Network/Collect-RuntimeDiagnostics.ps1`은 해당 PC의 로그와 배포 식별 정보를
`out/DiagnosticBundles`의 시각별 폴더·ZIP으로 복사한다. 스크립트만 다른 PC에 복사해도 실행할 수 있다.
portable 배포본에는 같은 수집기를 root에 두고 다음처럼 실행한다.

```powershell
powershell -NoProfile -ExecutionPolicy Bypass -File .\Collect-RuntimeDiagnostics.ps1 -RuntimeRoot .
```

`-RuntimeRoot`는 `-RepositoryRoot`의 별칭이며 생략하면 수집기 폴더를 사용한다.
`-Configuration Debug|Release`의 기본값은 Release다. root는 `Framework.sln`이 있는 저장소이거나,
schema `lostark.portable-runtime-bundle`, formatVersion 1, 같은 구성의 manifest와 Client/Server EXE가
있는 portable 폴더여야 한다. 실제 실행한 폴더를 지정하며 문제가 난 Client PC와 Server PC에서 각각 수집한다.

선택한 구성에서 client-session, server-session, server-send-progress, server-room-perf 종류별
최신 4개와 `.previous`, Client/Default의 Startup·Exit·EffectFailure·RendererExit 및 존재하는 회전본을 모은다.
Client.exe·Engine.dll·Server.exe의 크기·SHA256, Client/Server PID·경로·시작 시각은 `identity.json`에 기록한다.
bundle manifest, TeamLanEndpoint와 기존 설치 receipt·manifest가 있으면 함께 보존한다.
hash는 디스크 파일 기준이며 실행 중 메모리 이미지의 hash를 증명하지 않는다.
`collection.json`에는 복사한 파일 hash와 수집 실패를 기록한다.

수집기는 Client/Server를 시작·종료하거나 로그를 업로드하지 않는다.
Resources, 자격 증명, 환경 변수 전체는 수집하지 않는다. 로그에 없는 원격 PC의 상태를
로컬 실행 결과로 대신 확정하지 않고 같은 시각의 Client·Server 기록을 대조한다.
