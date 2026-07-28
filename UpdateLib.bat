@echo off
setlocal

set "BUILD_CONFIG=%~1"
if not defined BUILD_CONFIG set "BUILD_CONFIG=Debug"

if /i "%BUILD_CONFIG%"=="Debug" (
    set "ASSIMP_DLL=assimp-vc143-mtd.dll"
) else if /i "%BUILD_CONFIG%"=="Release" (
    set "ASSIMP_DLL=assimp-vc143-mt.dll"
) else (
    echo Unsupported configuration: %BUILD_CONFIG%
    exit /b 2
)

xcopy /y/s ".\Engine\Public\*.*" ".\EngineSDK\inc\" || exit /b 1
xcopy /y ".\Engine\Bin\*.lib" ".\EngineSDK\lib\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\*.lib" ".\EngineSDK\lib\" || exit /b 1
xcopy /y ".\Engine\Bin\Engine.dll" ".\Client\Bin\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\FMOD\Bin\fmod.dll" ".\Client\Bin\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\Assimp\Bin\%BUILD_CONFIG%\%ASSIMP_DLL%" ".\Client\Bin\" || exit /b 1
xcopy /y ".\Engine\Bin\ShaderFiles\*.*" ".\EngineSDK\hlsl\" || exit /b 1
xcopy /y ".\EngineSDK\hlsl\*.*" ".\Client\Bin\ShaderFiles\" || exit /b 1

echo Runtime dependencies deployed for %BUILD_CONFIG%.
exit /b 0
