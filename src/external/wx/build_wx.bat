
@SET build_wx=wx28
@SET build_msvc=10.0
@SET nmake_opts=

@REM set and check the wx build folder
@IF NOT "%1" == "" (
    @SET build_wx=%1
)

@SET build_wx=%build_wx%\build\msw

@IF NOT EXIST "%build_wx%" (
    @echo Directory %build_wx% not found
    GOTO END
)

@REM set and check the MSVC suite
@IF NOT "%2" == "" (
    @SET build_msvc=%2
)

@SET build_msvc="%PROGRAMFILES%\Microsoft Visual Studio %build_msvc%\VC\bin\vcvars32.bat"

@IF NOT EXIST %build_msvc% (
    @echo File %build_msvc% not found
    GOTO END
)

@REM set extra nmake optsnd check the MSVC suite
@IF NOT "%3" == "" (
    @SET nmake_opts=%3
)

@CALL %build_msvc%

@PUSHD %build_wx%

@SET OPTFLAGS="/O2 /Ob2 /Ot /Oy"
@REM /full optimize /always inline /optimize for speed /omit frame pointer

@SET BUILDFLAGS=VENDOR=csl UNICODE=1 MONOLITHIC=1 SHARED=1 RUNTIME_LIBS=static

@REM Unicode Debug Monolithic DLL
@nmake.exe -f makefile.vc BUILD=debug %BUILDFLAGS% %nmake_opts%

@REM Unicode Release Monolithic DLL
@nmake.exe -f makefile.vc BUILD=release %BUILDFLAGS% CXXFLAGS=%OPTFLAGS% CFLAGS=%OPTFLAGS% %nmake_opts%

@POPD

:END
