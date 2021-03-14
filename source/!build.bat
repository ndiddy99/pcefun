set TASMDIR=..\tools\tasm
set TASMTABS=%TASMDIR%
%TASMDIR%\tasm.exe -6280 -g3 -c -f0 main.asm main.pce main.lst
pause
