@echo off

set MRUBY_PACKAGE_DIR=%~dp0..

:top
shift
if "%0" equ "" goto :eof
if "%0" equ "--cc" goto cc
if "%0" equ "--cflags" goto cflags
if "%0" equ "--cxx" goto cxx
if "%0" equ "--cxxflags" goto cxxflags
if "%0" equ "--as" goto as
if "%0" equ "--asflags" goto asflags
if "%0" equ "--objc" goto objc
if "%0" equ "--objcflags" goto objcflags
if "%0" equ "--ld" goto ld
if "%0" equ "--ldflags" goto ldflags
if "%0" equ "--ldflags-before-libs" goto ldflagsbeforelibs
if "%0" equ "--libs" goto libs
if "%0" equ "--libmruby-path" goto libmrubypath
if "%0" equ "--help" goto showhelp
echo Invalid Option
goto :eof

:cc
echo cl.exe
goto top

:cflags
echo /nologo /W3 /MD /O2 /D_CRT_SECURE_NO_WARNINGS /we4013 /DMRB_STACK_EXTEND_DOUBLING /DMRB_UTF8_STRING /DMRB_USE_BIGINT /DMRB_USE_COMPLEX /DHAVE_MRUBY_ENCODING_GEM /DMRB_UTF8_STRING /DHAVE_MRUBY_IO_GEM /DMRB_USE_RATIONAL /I"%MRUBY_PACKAGE_DIR%\include" /I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-io\include" /I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-time\include"
goto top

:cxx
echo cl.exe
goto top

:cxxflags
echo /nologo /W3 /MD /O2 /D_CRT_SECURE_NO_WARNINGS /EHs /DMRB_STACK_EXTEND_DOUBLING /DMRB_UTF8_STRING /DMRB_USE_BIGINT /DMRB_USE_COMPLEX /DHAVE_MRUBY_ENCODING_GEM /DMRB_UTF8_STRING /DHAVE_MRUBY_IO_GEM /DMRB_USE_RATIONAL /I"%MRUBY_PACKAGE_DIR%\include" /I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-io\include" /I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-time\include"
goto top

:as
echo cc
goto top

:asflags
echo -D"MRB_UTF8_STRING" -D"MRB_USE_BIGINT" -D"MRB_USE_COMPLEX" -D"HAVE_MRUBY_ENCODING_GEM" -D"MRB_UTF8_STRING" -D"HAVE_MRUBY_IO_GEM" -D"MRB_USE_RATIONAL" -I"%MRUBY_PACKAGE_DIR%\include" -I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-io\include" -I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-time\include"
goto top

:objc
echo cc
goto top

:objcflags
echo -D"MRB_UTF8_STRING" -D"MRB_USE_BIGINT" -D"MRB_USE_COMPLEX" -D"HAVE_MRUBY_ENCODING_GEM" -D"MRB_UTF8_STRING" -D"HAVE_MRUBY_IO_GEM" -D"MRB_USE_RATIONAL" -I"%MRUBY_PACKAGE_DIR%\include" -I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-io\include" -I"%MRUBY_PACKAGE_DIR%\include\mruby\gems\mruby-time\include"
goto top

:ld
echo link.exe
goto top

:libs
echo libmruby.lib ws2_32.lib wsock32.lib ws2_32.lib
goto top

:ldflags
echo /NOLOGO /DEBUG /INCREMENTAL:NO /OPT:ICF /OPT:REF /DEBUG:NONE /LIBPATH:%MRUBY_PACKAGE_DIR%/lib
goto top

:ldflagsbeforelibs

goto top

:libmrubypath
echo %MRUBY_PACKAGE_DIR%/lib/libmruby.lib
goto top

:showhelp
echo Usage: mruby-config [switches]
echo   switches:
echo   --cc                       print C compiler name
echo   --cflags                   print flags passed to C compiler
echo   --cxx                      print C++ compiler name
echo   --cxxflags                 print flags passed to C++ compiler
echo   --as                       print assembler name
echo   --asflags                  print flags passed to assembler
echo   --objc                     print Objective C compiler name
echo   --objcflags                print flags passed to Objective C compiler
echo   --ld                       print linker name
echo   --ldflags                  print flags passed to linker
echo   --ldflags-before-libs      print flags passed to linker before linked libraries
echo   --libs                     print linked libraries
echo   --libmruby-path            print libmruby path
