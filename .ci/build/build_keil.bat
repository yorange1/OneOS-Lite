@ECHO OFF
SETLOCAL ENABLEDELAYEDEXPANSION
SETLOCAL
REM go to the folder where this bat script is located

WHERE /q UV4
IF ERRORLEVEL 1 (
    GOTO NOTINPATHERROR
)

FOR %%a IN ("%~dp0\.") DO FOR %%b IN ("%%~dpa\.") DO FOR %%c IN ("%%~dpb\.") DO SET PROJECT_DIR=%%~dpnxc\projects

ECHO Projects Directory: %PROJECT_DIR%

FOR /d %%I IN (%PROJECT_DIR%\*) DO (
    CALL UV4 -j0 -cr %%I\project.uvprojx -o %%I\keil_build.log
    IF !ERRORLEVEL! EQU 0 (
        ECHO Build %%I No Errors or Warnings
        DEL %%I\keil_build.log
    )
    IF !ERRORLEVEL! EQU 1 (
        TYPE %%I\keil_build.log
        DEL %%I\keil_build.log
        ECHO Build %%I Warnings Only
    )
    IF !ERRORLEVEL! EQU 2 (
        SET BUILD_ERRORLEVEL=!ERRORLEVEL!
        TYPE %%I\keil_build.log
        DEL %%I\keil_build.log
        ECHO Build %%I Errors
        GOTO BUILDFAILEDERROR
    )
)

ECHO Build all the project success
exit /b 0

:NOTINPATHERROR
ECHO Cannot find 'UV4' in your PATH. Install the Keil uVision5 before you continue
exit /b 1

:BUILDFAILEDERROR
exit /b %BUILD_ERRORLEVEL%
