@echo off

:: 1. 루트 폴더 삭제 (에러 메시지 숨김)
rd /s /q ".vs" 2>nul
rd /s /q "EngineSDK" 2>nul

:: 2. Bin 폴더 안의 파일들만 삭제
:: del /f /q "Client\Bin\*.*" 2>nul
del /f /q "Engine\Bin\*.*" 2>nul

:: 3. 특정 경로의 x64 폴더 삭제
rd /s /q "Client\Default\x64" 2>nul
rd /s /q "Engine\Default\x64" 2>nul

:: 작업 완료 후 바로 종료
exit