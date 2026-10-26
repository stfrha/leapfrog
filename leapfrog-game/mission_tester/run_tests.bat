@echo off
rem Runs the mission tests with the tester built by Visual Studio
rem (mission_tester project). Arguments go to the tester, e.g.
rem   run_tests.bat --filter "mission 1" --verbose
setlocal
set HERE=%~dp0
set EXE=%HERE%build\Release\mission_tester.exe
if not exist "%EXE%" set EXE=%HERE%build\Debug\mission_tester.exe
if not exist "%EXE%" (
   echo Build the mission_tester project in Visual Studio first.
   exit /b 2
)
"%EXE%" --root "%HERE%." %*
exit /b %ERRORLEVEL%
