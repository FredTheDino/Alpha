@echo off

mkdir bin
pushd bin

copy ..\include\win32\*.dll ..\bin

call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\bin\cl.exe" /Zi /DWINDOWS=1 ..\src\windows.cpp ..\include\win32\*.lib opengl32.lib /I ../include

popd