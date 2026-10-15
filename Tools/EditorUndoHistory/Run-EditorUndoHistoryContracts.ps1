[CmdletBinding()]
param(
    [ValidateSet('Debug','Release')][string]$Configuration = 'Debug',
    [string]$RepositoryRoot = '',
    [string]$HistoryRoot = '',
    [string]$VcVarsPath = '',
    [ValidatePattern('^[0-9.]+$')][string]$ToolsetVersion = '14.44',
    [ValidatePattern('^[0-9.]+$')][string]$WindowsSdkVersion = '10.0.26100.0'
)

# 이 파일은 C++ 테스트의 빌드·실행·기록을 조율한다. 검사 대상 알고리즘은 EditorUndoHistory.h이고 기대 결과는 Contracts.cpp가 정의한다.
# 제품 EXE 대신 out 아래의 별도 검사 EXE를 만들며 Client 실행이나 Data publish를 수행하지 않는다.
$ErrorActionPreference = 'Stop'
if (!$RepositoryRoot) { $RepositoryRoot = Join-Path $PSScriptRoot '../..' }
$repo = (Resolve-Path -LiteralPath $RepositoryRoot).Path
# 기본은 제품 헤더를 읽으며 HistoryRoot로 후보 헤더 경로를 지정할 수도 있다. C++ 테스트 소스 자체는 이 스크립트 옆 파일이다.
if (!$HistoryRoot) { $HistoryRoot = $repo }
$historySource = (Resolve-Path -LiteralPath $HistoryRoot).Path
$header = Join-Path $historySource 'Client/Public/EditorUndoHistory.h'
$sources = @(
    (Join-Path $PSScriptRoot 'EditorUndoHistoryContracts.cpp')
)
foreach ($inputPath in @($header) + $sources) {
    if (!(Test-Path -LiteralPath $inputPath)) { throw "Missing contract input: $inputPath" }
}
# 이미 선택한 Visual Studio 환경을 우선하고 없으면 설치된 C++ 도구를 찾는다. 실제 선택 결과는 아래 명령과 receipt에 남긴다.
if (!$VcVarsPath) {
    if ($env:VSINSTALLDIR) { $installation = $env:VSINSTALLDIR }
    else {
        $vswhere = Join-Path ${env:ProgramFiles(x86)} 'Microsoft Visual Studio/Installer/vswhere.exe'
        if (!(Test-Path -LiteralPath $vswhere)) { throw 'Specify -VcVarsPath or install the Visual Studio C++ toolchain.' }
        $installation = @(& $vswhere -latest -prerelease -products '*' -requires Microsoft.VisualStudio.Component.VC.Tools.x86.x64 -property installationPath)
        if ($LASTEXITCODE -or !$installation.Count) { throw 'No installed Visual Studio C++ toolchain was found.' }
        $installation = $installation[0]
    }
    $VcVarsPath = Join-Path $installation 'VC/Auxiliary/Build/vcvars64.bat'
}
$vcvars = (Resolve-Path -LiteralPath $VcVarsPath).Path
# 실행별 시각/PID로 격리 폴더를 만들고 컴파일러·검사 로그와 소스 hash를 함께 보관한다. 이 폴더가 제품 배포본은 아니다.
$stamp = [DateTime]::UtcNow.ToString('yyyyMMddTHHmmssfffZ')
$output = Join-Path $repo "out/EditorUndoHistory/$stamp-$Configuration-$PID"
New-Item -ItemType Directory -Path $output | Out-Null
# cmd에 전달할 경로에서 명령 인용/확장을 깨는 문자를 거절한다. 사용자 데이터 문자열을 검사 실행 인자로 조립하지 않는다.
foreach ($inputPath in @($repo, $historySource, $PSScriptRoot, $vcvars, $output)) {
    if ($inputPath.IndexOfAny([char[]]@('"', "`r", "`n", '%', '!')) -ge 0) {
        throw "Unsupported command path: $inputPath"
    }
}

# 실제 C++20 헤더를 MSVC로 컴파일한다. 이 스크립트가 같은 Undo 알고리즘을 PowerShell로 다시 구현하는 방식이 아니다.
$flags = @('/nologo','/EHsc','/std:c++20','/utf-8','/wd4828','/W4','/Od','/sdl','/permissive-',
    '/DNOMINMAX','/D_UNICODE','/DUNICODE','/D_WINDOWS')
if ($Configuration -eq 'Debug') { $flags += @('/MDd','/D_DEBUG','/D_ITERATOR_DEBUG_LEVEL=2') }
else { $flags += @('/MD','/DNDEBUG','/D_ITERATOR_DEBUG_LEVEL=0') }
$executable = Join-Path $output 'EditorUndoHistoryContracts.exe'
# 후보 헤더 검색 경로를 먼저 두고 제품 경로를 뒤에 둔다. 결과 OBJ/PDB/EXE는 실행별 out 디렉터리에만 쓴다.
$arguments = $flags + @(
    '/I"' + (Join-Path $historySource 'Client/Public') + '"',
    '/I"' + (Join-Path $repo 'Client/Public') + '"',
    '/Fo"' + $output + '/"',
    '/Fd"' + (Join-Path $output 'compiler.pdb') + '"',
    '/Fe"' + $executable + '"'
)
$arguments += @($sources | ForEach-Object { '"' + $_ + '"' })
$arguments += @('/link', '/INCREMENTAL:NO')
$utf8 = [Text.UTF8Encoding]::new($false)
$response = Join-Path $output 'compile.rsp'
[IO.File]::WriteAllText($response, ($arguments -join "`r`n") + "`r`n", $utf8)
$command = 'call "' + $vcvars + '" ' + $WindowsSdkVersion + ' -vcvars_ver=' + $ToolsetVersion + ' >nul && cl.exe @"' + $response + '"'
# 빌드 직전 입력 hash를 기록한다. 실행 중 편집까지 막는 lock/CAS는 아니므로 결과 해석 시 검증했던 저장본과 현재 소스를 구분한다.
$receipt = [ordered]@{
    configuration = $Configuration
    startedUtc = [DateTime]::UtcNow.ToString('o')
    compilerExitCode = $null
    testExitCode = $null
    command = $command
    sources = @(@($header) + $sources | ForEach-Object {
        [ordered]@{ path = $_; sha256 = (Get-FileHash -LiteralPath $_ -Algorithm SHA256).Hash.ToLowerInvariant() }
    })
}
$oldPreference = $ErrorActionPreference
try {
    $ErrorActionPreference = 'Continue'
    # 외부 프로세스의 출력과 종료 코드를 따로 기록한다. 컴파일 성공 뒤에만 검사 EXE를 실행한다.
    & $env:COMSPEC /d /v:off /s /c $command *> (Join-Path $output 'compile.log')
    $receipt.compilerExitCode = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    Get-Content -LiteralPath (Join-Path $output 'compile.log') -Tail 30
    if ($receipt.compilerExitCode) { throw "Editor history contract compilation failed: $($receipt.compilerExitCode)" }
    $ErrorActionPreference = 'Continue'
    # 여기서 실행하는 것은 값 이력 테스트다. 실패 종료 코드가 오면 성공 메시지를 남기지 않고 예외로 끝낸다.
    & $executable $output *> (Join-Path $output 'run.log')
    $receipt.testExitCode = $LASTEXITCODE
    $ErrorActionPreference = $oldPreference
    Get-Content -LiteralPath (Join-Path $output 'run.log')
    if ($receipt.testExitCode) { throw "Editor history contracts failed: $($receipt.testExitCode)" }
    Write-Output "Editor history contract evidence: $output"
}
# 컴파일 또는 검사 실패도 현재까지의 종료 코드와 입력 정보를 남긴다. 초기 경로/도구 탐색 실패는 이 try 범위 밖이다.
finally {
    $ErrorActionPreference = $oldPreference
    [IO.File]::WriteAllText((Join-Path $output 'receipt.json'), ($receipt | ConvertTo-Json -Depth 5), $utf8)
}
