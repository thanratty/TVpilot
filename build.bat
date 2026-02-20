
choice /C DR /m "Choose [D]ebug or [R]elease : "

if %ERRORLEVEL% EQU 1 GOTO do_debug

:do_release

msbuild TVpilot.sln /t:ReBuild /p:Platform=x86 /property:Configuration="Release"
exit /b

:do_debug

msbuild TVpilot.sln /t:ReBuild /p:Platform=x86 /property:Configuration="Debug"
exit /b






