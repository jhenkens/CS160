#!/bin/sh
testcount=0
make clean >/dev/null
make CPPFLAGS="-DTESTING" > /dev/null
echo "Testing folding..."
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
make clean >/dev/null
make CPPFLAGS="-DTESTING -DNOFOLDING" >/dev/null
echo "Testing non-folded..."
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
done

echo "Completed $testcount tests successfully for both folded and non-folded!"

