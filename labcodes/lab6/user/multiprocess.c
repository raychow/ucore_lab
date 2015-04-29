#include <ulib.h>
#include <stdio.h>

const int max_child = 32;

int
main(void) {
    int n, pid;
    for (n = 0; n < max_child; n ++) {
        if ((pid = fork()) == 0) {
            int i = 0;
            while (1) {
                if (10000000 == ++i) {
                    cprintf("I am child %d\n", n);
                    i = 0;
                }
            }
        }
        assert(pid > 0);
    }
    return 0;
}

