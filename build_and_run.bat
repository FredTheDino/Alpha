@echo off
del bin /Q

call path.bat
call build.bat

echo -
echo ============ Done Building
echo -

call bin\windows.exe
pause