#!/bin/sh
testcount=0
for file in in/*; do
    ./simple < $file > simple.s
    gcc -m32 -c -o simple.o simple.s
    gcc -m32 -c -o start.o start.c
    gcc -m32 -o start start.o simple.o
    if ! ./start | cmp - out/$(basename $file)
    then
        echo "Expected and output differ for test $file"
        break
    fi
    let "testcount+=1"
done

echo "Completed $testcount tests successfully!"

