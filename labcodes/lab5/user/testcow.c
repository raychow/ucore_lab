#include <stdio.h>
#include <ulib.h>

#define ARRAYSIZE (1024*3)
const int max_child = 32;

uint32_t array[ARRAYSIZE];

int
main(void) {
    int n, pid;
    array[1] = 1;
    for (n = 0; n < max_child; n ++) {
        if ((pid = fork()) == 0) {
            array[2] = n;
            array[1024] = 1024;
            array[2048] = 2048;
            cprintf("1st test, child %d, array[1] = %d, array[2] = %d, array[3] = %d, array[1024] = %d, array[2048] = %d\n",
                    n, array[1], array[2], array[3], array[1024], array[2048]);
            array[3] = n;
            array[1024] = 1000;
            array[2048] = 2000;
            exit(0);
        }
        assert(pid > 0);
    }

    array[1] = 111;
    array[2] = 222;
    array[1024] = 1111;
    array[2048] = 2222;
    cprintf("1st test, parent, array[1] = %d, array[2] = %d, array[3] = %d, array[1024] = %d, array[2048] = %d\n",
            array[1], array[2], array[3], array[1024], array[2048]);
    array[3] = 333;

    for (n = 0; n < max_child; n ++) {
        if ((pid = fork()) == 0) {
            cprintf("2nd test, child %d, array[1] = %d, array[2] = %d, array[3] = %d, array[1024] = %d, array[2048] = %d\n",
                    n, array[1], array[2], array[3], array[1024], array[2048]);
            exit(0);
        }
        assert(pid > 0);
    }

    cprintf("2nd test, parent, array[1] = %d, array[2] = %d, array[3] = %d, array[1024] = %d, array[2048] = %d\n",
            array[1], array[2], array[3], array[1024], array[2048]);
}

