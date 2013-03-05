
./simple < sample.smpl > sample.s
#gcc -m32 -c -o start.o start.c
gcc -Wa,--32 -m32 -c -o sample.o sample.s
gcc -m32 -o sample start.o sample.o

