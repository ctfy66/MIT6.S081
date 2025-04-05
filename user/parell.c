#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"

int main() {
    if (fork() == 0) {
        printf("child");
    } else {
        printf("parent");
    }
    return 0;
}