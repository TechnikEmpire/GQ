@echo off

setlocal enabledelayedexpansion

:: Set GUMBO_VERSION to the extracted folder name, which includes the 
:: version. So, as an example, in 
:: STAHPIT_ROOT_DIR\libstahp\deps\gumbo-parser, you should have extracted 
:: lets say Gumbo Parser 0.10.1. So you'll have 
:: STAHPIT_ROOT_DIR\libstahp\deps\gumbo-parser\gumbo-parser-0.10.1\SOURCES. 
:: Therefore, set GUMBO_VERSION to "gumbo-parser-0.10.1" like so:
:: GUMBO_VERSION=gumbo-parser-0.10.1
set GUMBO_VERSION=

:: Make sure the variables have been set.
if [%GUMBO_VERSION%] == [] GOTO RequiresSetup

:: Invoke the 64 bit Visual Studio command prompt environment, for 64 
:: bit build tool stuff to be setup. If you have installed VS 2015 to a 
:: non-standard place, or have an older version of VS, you will need to 
:: change this line. 
call "C:\Program Files (x86)\Microsoft Visual Studio 14.0\VC\vcvarsall.bat" amd64

:: Change the current directory to the gumbo-parser source directory.
cd ..\deps\gumbo-parser\%GUMBO_VERSION%\src

:: Delete any artifacts from previous builds.
del /F %cd%\*.dll
del /F %cd%\*.lib
del /F %cd%\obj\*.obj

:: Build a list of all source files for passing to the compiler.
for /f %%i in ('dir /B %cd%\*.c') do set SOURCES=%%i !SOURCES!

:: Build a list of all expected output obj files, for passing to the linker.
for /f %%j in ('dir /B %cd%\*.c') do set OBJS=%cd%\obj\%%~nj.obj !OBJS!

:: Build and link.
mkdir obj
cl.exe /nologo /D WIN32 /D DEBUG /D _CONSOLE /Fo%cd%\obj\ /MDd /LD /I %cd% /I %cd%\..\visualc\include %SOURCES% /link /DLL /MACHINE:x64 /OUT:gumbo_parser.dll

:: Generate corresponding LIB file.
LIB /NOLOGO /MACHINE:x64 /OUT:gumbo_parser.lib %OBJS%

:: Generate output directories. If they exist, no harm, no errors.
mkdir ..\..\msvc
mkdir "..\..\msvc\Debug x64"
mkdir "..\..\msvc\Debug x64\include"
mkdir "..\..\msvc\Debug x64\lib"

:: Copy the build output (dll and lib files), as well as the headers to 
:: the generated output directories. 
xcopy /Y *.dll "..\..\msvc\Debug x64\lib"
xcopy /Y *.lib "..\..\msvc\Debug x64\lib"
xcopy /Y *.h "..\..\msvc\Debug x64\include"
xcopy /Y "%cd%\..\visualc\include\*.h" "..\..\msvc\Debug x64\include"

:: Change back to the scripts directory.
cd %cd%\..\..\..\..\scripts

:: Quit
exit /B

:: This will print out help information to the console to assist the 
:: user in correctly configuring the script in the event that the variables 
:: were not setup. 
:RequiresSetup
echo. & echo Please edit this batch file to have the following variables set correctly: & echo.
echo 	GUMBO_VERSION - Set this to the name of the Gumbo Parser source directory, the folder which includes the version number in it. & echo.
echo. & echo See script comments for more details.
timeout 10
exit /B