// mycat3.c
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <stdint.h>

// 获取系统页大小
static size_t get_page_size(void) {
    long ps = sysconf(_SC_PAGESIZE);
    if (ps <= 0) {
        perror("sysconf");
        return 4096;
    }
    return (size_t)ps;
}

// 分配对齐到页边界的内存
// 返回值 ptr 必须通过 align_free 释放
char *align_alloc(size_t size) {
    size_t align = get_page_size();
    // 多分配 align + sizeof(void*) 用来存储原始指针
    void *orig = malloc(size + align + sizeof(void*));
    if (!orig) return NULL;
    uintptr_t raw = (uintptr_t)orig + sizeof(void*);
    // 向上对齐到 align
    uintptr_t aligned = (raw + align - 1) & ~(align - 1);
    // 在 aligned 地址前面存回原始指针
    ((void**)aligned)[-1] = orig;
    return (char*)aligned;
}

// 释放由 align_alloc 返回的指针
void align_free(void *ptr) {
    if (!ptr) return;
    void *orig = ((void**)ptr)[-1];
    free(orig);
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *msg = "Usage: mycat3 <file>\n";
        write(STDERR_FILENO, msg, 22);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    size_t bufsize = get_page_size();
    char *buf = align_alloc(bufsize);
    if (!buf) {
        perror("align_alloc");
        close(fd);
        return 1;
    }

    ssize_t nr;
    while ((nr = read(fd, buf, bufsize)) > 0) {
        ssize_t nw;
        char *outp = buf;
        while (nr > 0) {
            nw = write(STDOUT_FILENO, outp, (size_t)nr);
            if (nw < 0) {
                perror("write");
                align_free(buf);
                close(fd);
                return 1;
            }
            outp += nw;
            nr   -= nw;
        }
    }
    if (nr < 0) {
        perror("read");
    }

    align_free(buf);
    close(fd);
    return (nr < 0) ? 1 : 0;
}
