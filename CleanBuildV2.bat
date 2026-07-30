@echo off
setlocal

rem Remove generated IDE and SDK state.
rd /s /q ".vs" 2>nul
rd /s /q "EngineSDK" 2>nul

rem Remove configuration-specific binaries without touching Resources, DataFiles, or ShaderFiles.
rd /s /q "Client\Bin\Debug" 2>nul
rd /s /q "Client\Bin\Release" 2>nul
rd /s /q "Engine\Bin\Debug" 2>nul
rd /s /q "Engine\Bin\Release" 2>nul

rem Remove legacy shared output files left by the previous layout.
del /f /q "Client\Bin\*.exe" "Client\Bin\*.dll" "Client\Bin\*.pdb" "Client\Bin\*.ilk" 2>nul
del /f /q "Engine\Bin\*.dll" "Engine\Bin\*.lib" "Engine\Bin\*.pdb" 2>nul

rem Remove compiler and linker intermediate outputs.
rd /s /q "Client\Default\x64" 2>nul
rd /s /q "Engine\Default\x64" 2>nul

exit /b 0
