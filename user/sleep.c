#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main(int argc, char* argv[]) {
    if (argc == 1) {
        write(1, "err: missing argument.\n", 23);
        exit(1);
    }

    //using atoi to convert char to integer.
    int ticks = atoi(argv[1]);

    //call syscall sleep
    sleep(ticks);

    exit(0);
}