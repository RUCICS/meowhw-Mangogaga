// mycat2.c
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>

// 获取 I/O 缓冲区大小（系统页大小）
static size_t io_blocksize(void) {
    long ps = sysconf(_SC_PAGESIZE);
    if (ps <= 0) {
        perror("sysconf");
        // 如果获取失败，退回到一个安全的默认值
        return 4096;
    }
    return (size_t)ps;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *msg = "Usage: mycat2 <file>\n";
        write(STDERR_FILENO, msg, 22);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    size_t bufsize = io_blocksize();
    char *buf = malloc(bufsize);
    if (!buf) {
        perror("malloc");
        close(fd);
        return 1;
    }

    ssize_t nr;
    while ((nr = read(fd, buf, bufsize)) > 0) {
        char *outp = buf;
        ssize_t nw;
        while (nr > 0) {
            nw = write(STDOUT_FILENO, outp, (size_t)nr);
            if (nw < 0) {
                perror("write");
                free(buf);
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

    free(buf);
    close(fd);
    return (nr < 0) ? 1 : 0;
}
