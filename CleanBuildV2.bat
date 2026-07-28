@echo off

:: 1. 루트 폴더 삭제
rd /s /q ".vs" 2>nul
rd /s /q "EngineSDK" 2>nul

:: 2. Client\Bin 폴더 - assimp DLL 2개 제외하고 삭제
if exist "Client\Bin" (
    for %%f in ("Client\Bin\*") do (
        set "skip="
        if /i "%%~nxf"=="assimp-vc143-mtd.dll" set "skip=1"
        if /i "%%~nxf"=="assimp-vc143-mt.dll" set "skip=1"
        
        if not defined skip (
            del /f /q "%%f" 2>nul
        )
    )
)

:: 3. Engine\Bin 폴더 삭제 (여기도 제외가 필요하면 위와 동일하게 수정 가능)
del /f /q "Engine\Bin\*.*" 2>nul

:: 4. 특정 경로의 x64 폴더 삭제
rd /s /q "Client\Default\x64" 2>nul
rd /s /q "Engine\Default\x64" 2>nul

:: 작업 완료 후 종료
exit