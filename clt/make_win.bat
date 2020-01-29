@echo off

for /f %%i in ('hostname') do set CURRENT_HOST=%%i
set CURRENT_USER=%USERNAME%@%CURRENT_HOST%
for /f %%i in ('date /t') do set CURRENT_DATE=%%i
for /f %%i in ('git rev-parse HEAD') do set GIT_COMMIT=%%i

echo Username@Host is: %CURRENT_USER%
echo Current date was set to: %CURRENT_DATE%
echo Git commit was set to: %GIT_COMMIT%

mingw32-make -j8 -f Makefile.windows
