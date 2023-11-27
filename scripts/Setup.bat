@echo off

pushd %~dp0\..
Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build.lua vs2022
Walnut\vendor\bin\premake\Windows\premake5.exe --file=Build-Headless.lua vs2022
popd
pause
