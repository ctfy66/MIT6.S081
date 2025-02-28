#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"

int main() {
    int p2c[2]; //pipe for parent to child
    int c2p[2]; //pipe for child to parent
    pipe(p2c);
    pipe(c2p);
    int pid = fork();
    if (pid < 0) {
        fprintf(2, "fork failed.");
        exit(1);
    }

    if (pid > 0) {
        //parent process
        close(p2c[0]);
        close(c2p[1]);
        write(p2c[1], "ping", 4);
        char buf[5];
        read(c2p[0], buf, 4);
        fprintf(1, "%d: received %s\n", getpid(), buf);
        close(c2p[0]);
        close(p2c[1]);
        wait(0);
    } else {
        //child process
        close(p2c[1]);
        close(c2p[0]);
        char buf[5];
        read(p2c[0], buf, 4);
        fprintf(1, "%d: received %s\n", getpid(), buf);
        close(p2c[0]);
        write(c2p[1], "pong", 4);
        close(c2p[1]);
        exit(0);
    }
    exit(0);
}