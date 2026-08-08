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

set "ENGINE_BIN=.\Engine\Bin\%BUILD_CONFIG%"
set "ENGINE_SDK_LIB=.\EngineSDK\lib\%BUILD_CONFIG%"
set "CLIENT_BIN=.\Client\Bin\%BUILD_CONFIG%"

if not exist "%ENGINE_BIN%\Engine.lib" (
    echo Engine import library was not built for %BUILD_CONFIG%: %ENGINE_BIN%\Engine.lib
    exit /b 1
)
if not exist "%ENGINE_BIN%\Engine.dll" (
    echo Engine runtime was not built for %BUILD_CONFIG%: %ENGINE_BIN%\Engine.dll
    exit /b 1
)
if /i "%BUILD_CONFIG%"=="Debug" if not exist "%ENGINE_BIN%\Engine.pdb" (
    echo Engine debug symbols were not built: %ENGINE_BIN%\Engine.pdb
    exit /b 1
)

xcopy /y/s ".\Engine\Public\*.*" ".\EngineSDK\inc\" || exit /b 1
xcopy /y "%ENGINE_BIN%\*.lib" "%ENGINE_SDK_LIB%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\*.lib" ".\EngineSDK\lib\" || exit /b 1
xcopy /y "%ENGINE_BIN%\Engine.dll" "%CLIENT_BIN%\" || exit /b 1
if /i "%BUILD_CONFIG%"=="Debug" (
    xcopy /y "%ENGINE_BIN%\Engine.pdb" "%CLIENT_BIN%\" || exit /b 1
)
xcopy /y ".\Engine\ThirdPartyLib\FMOD\Bin\fmod.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\Assimp\Bin\%BUILD_CONFIG%\%ASSIMP_DLL%" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysX_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysXCommon_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\ThirdPartyLib\PhysX\Bin\%BUILD_CONFIG%\PhysXFoundation_64.dll" "%CLIENT_BIN%\" || exit /b 1
xcopy /y ".\Engine\Bin\ShaderFiles\*.*" ".\EngineSDK\hlsl\" || exit /b 1
xcopy /y ".\EngineSDK\hlsl\*.*" ".\Client\Bin\ShaderFiles\" || exit /b 1

echo Runtime dependencies deployed to %CLIENT_BIN% for %BUILD_CONFIG%.
exit /b 0
