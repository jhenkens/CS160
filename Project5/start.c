#include <stdio.h>

int Main();  // note that Main now returns an integer!

int main(int argc, char **argv) {
    int i = Main();
    printf("%d\n", i);
    return 0;
}