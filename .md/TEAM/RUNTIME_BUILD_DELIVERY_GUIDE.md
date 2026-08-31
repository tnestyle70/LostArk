# Runtime 빌드 ZIP·Drive Resources 전달 가이드

## 경계

팀 Git은 코드와 `Data` 저작 정본을 관리하고, 팀장 Drive는
`Client/Bin/Resources` 물리 리소스를 관리한다. Runtime ZIP은 이미 빌드·publish된 EXE, DLL,
CSO와 필요한 `Client/Server Bin/DataFiles`만 전달한다. Resource 전체를 ZIP, manifest 또는 Git
정본으로 승격하지 않는다.

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

먼저 같은 commit에서 필요한 Product/Core 빌드를 통과시킨다. 그 뒤 다음처럼 ZIP을 만든다.

```powershell
powershell -ExecutionPolicy Bypass -File Tools/ResourceDelivery/New-LostArkRuntimeDelivery.ps1 `
  -Configuration Debug `
  -RepositoryRoot C:\Users\user\Desktop\LostArk `
  -OutputZip C:\전달\LostArk-Debug-Runtime.zip `
  -IncludePdb
```

ZIP에는 `Client/Bin/Resources`가 0개여야 한다. manifest의 per-file hash는 전송 중 손상과
경로 변조를 설치 전에 거부하기 위한 것이며, Resource pack version/lock 계약이 아니다.

## 받는 PC

1. Git에서 보내는 PC와 같은 commit을 checkout한다.
2. 팀장 Drive의 `Client/Bin/Resources`를 원래 상대 경로에 둔다.
3. 아래 설치 스크립트에 ZIP과 실제 LostArk 물리 폴더를 전달한다.

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

Debug 저작 기능은 Server + Client profile에서 사용자가 직접 확인한다. Release에서는 F1/Workbench가
노출되지 않으므로 제품 Lobby/Level 진입만 확인한다. 화면과 음향 fidelity는 자동 설치 결과가 아니라
사용자의 수동 smoke 판정이다.
