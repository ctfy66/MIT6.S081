#include "kernel/types.h"
#include "kernel/stat.h"
#include "user.h"
void primes(int fdRead) {
    int prime;
    if (read(fdRead,&prime,sizeof(int)) <= 0) {
        fprintf(2, "cannot read from pipe.");
        exit(1);
    }
    fprintf(1, "prime %d\n", prime);
    if (prime == 31) {
        exit(0);
    }
    int pp[2];
    pipe(pp);
    if (fork() == 0) {
        close(pp[0]);
        int num;
        while (read(fdRead, &num, sizeof(int)) > 0)
        {
            if (num % prime == 0) continue;
            write(pp[1], &num, sizeof(int));
        }
        close(pp[1]);
        close(fdRead);
        wait(0);
    } else {
        close(pp[1]);
        primes(pp[0]);
    }
    exit(0);
}


int main(int argc, char* argv[]) {
    int fd[2];
    pipe(fd);
    for (int i = 2; i < 36; i++) {
        write(fd[1], &i, sizeof(int));
    }
    close(fd[1]);
    primes(fd[0]);
    exit(0);
}