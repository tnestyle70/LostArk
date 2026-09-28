@echo off
setlocal
start "" /wait "%~dp0LostArk.exe" --server
exit /b %errorlevel%
