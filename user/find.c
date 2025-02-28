//
// Created by ctfy on 25-2-28.
//
#include "kernel/types.h"
#include "kernel/stat.h"
#include "user/user.h"
#include "kernel/fs.h"
/*
void findTarget(char* path) {
    int fd;
    struct stat st;
    struct dirent de;
    char* p, buf[512];
     if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }
    switch (st.type) {
        case T_FILE:
            fprintf(1, path);
            break;
        case T_DIR:
            fprintf(1, path);
            strcpy(buf, path);
            p = buf + strlen(buf);
            // 在路径末尾加上 /, p加1依然指向路径末尾。
            *p++ = '/';

            while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            p[DIRSIZ] = 0;
            findTarget(buf);
            }
            break;
            
    }
    close(fd);

}
*/
void find(char* path, char* target) {
    char buf[512], *p;
    int fd;
    struct stat st;
    struct dirent de;
    //call open() to open flie or directory
    if ((fd = open(path, 0)) < 0) {
        fprintf(2, "find: cannot open %s\n", path);
        return;
    }

    //call fstat() to get file or directory`s state
    if (fstat(fd, &st) < 0) {
        fprintf(2, "find: cannot stat %s\n", path);
        close(fd);
        return;
    }

    switch(st.type) {
        case T_FILE : //当前文件如果是file类型
            p = path + strlen(path);
            while (p >= path && *p != '/')
            {
                p--;
            }
            p++; // p point to filename
            if (strcmp(p, target) == 0) {
                fprintf(1, "%s\n", path);
            }
            break;
        case T_DIR :
        // directory type
        strcpy(buf, path);
        p = buf + strlen(buf);
        // 在路径末尾加上 /, p加1依然指向路径末尾。
        *p++ = '/';

        while (read(fd, &de, sizeof(de)) == sizeof(de)) {
            if (de.inum == 0)
                continue;
            memmove(p, de.name, DIRSIZ);
            if (strcmp(de.name, ".") == 0 || strcmp(de.name, "..") == 0) {
                continue;
            }
            p[DIRSIZ] = 0;
            find(buf, target);
        }
        break;

    }
    close(fd);

}

int main(int argc, char* argv[]) {
    if (argc < 3) {
        fprintf(2, "err: missing argument.");
        exit(1);
    }
    find(argv[1], argv[2]);
    exit(0);
}