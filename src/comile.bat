make -j profile-build ARCH=x86-64-modern COMP=mingw
strip stockfish.exe
mv stockfish.exe ../../stockfish_x86-64-modern.exe
make clean
cd