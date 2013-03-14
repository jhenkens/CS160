#!/bin/sh
testcount=0
if ! $( make clean >/dev/null && make CPPFLAGS="-DTESTING" > /dev/null)
then
    echo "Failed to make"
    exit 4
fi
echo "Testing folding..."
for file in in/*; do
    ./simple < $file > simple.s
    gcc -m32 -c -o simple.o simple.s
    gcc -m32 -c -o start.o start.c
    gcc -m32 -o start start.o simple.o
    if ! ./start | cmp - out/$(basename $file)
    then
        echo "Expected and output differ for test $file"
        exit 2
    fi
    let "testcount+=1"
done

if ! $( make clean >/dev/null && make CPPFLAGS="-DTESTING -DNOFOLDING" > /dev/null)
then
    echo "Failed to make"
    exit 4
fi
echo "Testing non-folded..."
for file in in/*; do
    ./simple < $file > simple.s
    gcc -m32 -c -o simple.o simple.s
    gcc -m32 -c -o start.o start.c
    gcc -m32 -o start start.o simple.o
    if ! ./start | cmp - out/$(basename $file)
    then
        echo "Expected and output differ for test $file"
        exit 3
    fi
done

echo "Completed $testcount tests successfully for both folded and non-folded!"

