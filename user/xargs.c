#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
#include "kernel/param.h"
int main(int argc, char* argv[]) {
    if (argc < 2) {
        fprintf(2, "usage:xargs command [argument]\n");
        exit(1);
    }

    //create argv array
    char* newArgv[MAXARG];
    for (int i = 0; i < argc - 1; i++) {
        newArgv[i] = argv[i + 1];
    }
    char c;
    char* p;
    char buf[512];
    p = buf;
    while (read(0, &c, 1) > 0)
    {
        if (c == '\n') {
            *p = 0;
            newArgv[argc - 1] = buf;
            newArgv[argc] = 0;
            p = buf;
            if (fork() == 0) {
                exec(newArgv[0], newArgv);
                fprintf(2, "exec %s failed\n", newArgv[0]);
                exit(1);
            }
            wait(0);
        } else {
            *p = c;
            p++;
        }
    }
    if (p != buf) {
        *p = 0;
        newArgv[argc - 1] = buf;
        newArgv[argc] = 0;
        p = buf;
        if (fork() == 0) {
            exec(newArgv[0], newArgv);
            fprintf(2, "exec %s failed\n", newArgv[0]);
            exit(1);
        }
        wait(0);
    }
    exit(0);
    
    
}