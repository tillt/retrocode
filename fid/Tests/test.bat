@echo off
REM RetroFID(tm) testing script V0.1
REM -----------------------------------------------
REM supply parameter 'd' for debug-version testing
REM -----------------------------------------------
SET test_pattern=*.*
SET input_dir=Input\
SET output_txt=output.txt
SET bin_dir=..\..\Release
IF d==%1% SET bin_dir=..\..\Debug
IF EXIST %output_txt% DEL %output_txt%
FOR /f %%a IN ('dir /b %input_dir%\%test_pattern%') DO run.bat %input_dir% %%a %output_txt% %bin_dir%
