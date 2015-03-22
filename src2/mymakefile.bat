make profile-build ARCH=x86-64 COMP=mingw
pause
strip stockfish.exe
pause
stockfish bench
pause
del /q *.o
pause
set HR=%time:~0,2%
set HR=%Hr: =0% 
set HR=%HR: =%
rename stockfish.exe sf_%date:~10,4%-%date:~4,2%-%date:~7,2%_%HR%%time:~3,2%.exe
exit 0