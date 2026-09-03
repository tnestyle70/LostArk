@echo off
rem One-shot: project Valtan split Products, run canonical domain validation /
rem publish, then build and run regression. Arguments pass through unchanged:
rem   RunFullPipeline.bat                      Debug / FullDiagnostic
rem   RunFullPipeline.bat -Profile Product     Debug / Product only
rem   RunFullPipeline.bat -DataOnly            publish only, no compile
rem   RunFullPipeline.bat -Configuration Release
setlocal
pushd "%~dp0"
powershell -NoProfile -ExecutionPolicy Bypass -File "Tools\Build\Run-FullPipeline.ps1" %*
set "EXIT_CODE=%ERRORLEVEL%"
popd
exit /b %EXIT_CODE%
