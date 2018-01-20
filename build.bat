@echo off

set EXE_NAME=godnatt.exe

if "%1" equ "clean" (
    echo Cleaning...
    rmdir /S /Q bin
    goto END
)

if not defined DEV_ENV_DIR (
    call "%VSVC%\vcvarsall.bat" x64
)
set DEV_ENV_DIR= ???


set RELEASE_MODE=0

if "%1" equ "release" ( rem C:\>build release
    set RELEASE_MODE=1
)

:: -FC      displays full path of source files
:: -EHsc-   turns off exceptions
:: -GR-     turn of rtti, which we can since we don't use dynamic_cast or typeid, smaller executable
:: -opt:ref dead code elimination
set CFLAGS= -nologo -FC -EHsc- -GR-
set LFLAGS= -incremental:no -opt:ref 

set LIBS= kernel32.lib user32.lib taskschd.lib comsupp.lib

if %RELEASE_MODE% equ 0 ( rem debug
    set CFLAGS=%CFLAGS% -Od -MDd -Zi -DDEBUG
    set LFLAGS=%LFLAGS%  -SUBSYSTEM:CONSOLE -debug
    echo Building debug...
) else (                  rem release
    :: statically link standard library which I hope makes it so vc++2015-redist doesn't have to be installed
    set CFLAGS=%CFLAGS% /O2 /MT
    set LFLAGS=%LFLAGS% /ENTRY:"mainCRTStartup" /SUBSYSTEM:WINDOWS
    echo Building release...
)

set WARNINGS= /Wall /WX /wd4514 /wd4710 /wd4711 /wd4820

if %RELEASE_MODE% equ 0 ( rem debug
    :: Unreferenced formal parameter and local variables
    set WARNINGS=%WARNINGS% /wd4100 /wd4101 /wd4189
)

if not exist .\bin mkdir .\bin
pushd .\bin

cl %CFLAGS% %WARNINGS% ..\src\godnatt.cpp /Fe:%EXE_NAME% /link %LIBS% %LFLAGS%

popd

echo Done!

:END