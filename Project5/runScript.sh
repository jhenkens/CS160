#!/bin/sh
./simple < $1 > simple.s
gcc -m32 -c -o simple.o simple.s
gcc -m32 -c -o start.o start.c
gcc -m32 -o start start.o simple.o
./start
