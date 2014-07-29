@echo off
setlocal 

rem Change the current directory to the directory where the batch is located
pushd %~dp0

rem Doxygen starts one directory up from here
cd ..

rem Try to find Doxygen
SET "DOXYGEN_EXE=MISSING"

rem Tests to see if doxygen is in the path
rem Thanks to Raymon Chen for this one http://blogs.msdn.com/b/oldnewthing/archive/2005/01/20/357225.aspx
rem
rem Tries to find Doxygen in the PATH variable
for %%i in (doxygen.exe) do @if NOT "%%~$PATH:i"=="" SET "DOXYGEN_EXE=%%~$PATH:i"

IF "%DOXYGEN_EXE%"=="MISSING" (
    rem This is to be backward compatible with the original build script
    IF EXIST bin\win32\dev\doxygen.exe (
        SET "DOXYGEN_EXE=bin\win32\dev\doxygen.exe"
    ) ELSE (
        echo Documentation not generated (could not find doxygen.exe in your path)
        goto cleanup
    )
)

rem Run doxygen
"%DOXYGEN_EXE%"

:cleanup

popd
endlocal
