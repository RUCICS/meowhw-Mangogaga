// mycat5.c
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdint.h>
#include <unistd.h>

// 获取系统页大小或文件系统推荐块大小
static size_t io_blocksize(int fd) {
    struct stat st;
    if (fstat(fd, &st) == 0 && st.st_blksize > 0) {
        return (size_t)st.st_blksize;
    }
    long ps = sysconf(_SC_PAGESIZE);
    if (ps > 0) {
        return (size_t)ps;
    }
    return 4096;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *msg = "Usage: mycat5 <file>\n";
        write(STDERR_FILENO, msg, 17);
        return 1;
    }

    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // Task5: buffer = 8 × base blocksize
    size_t base = io_blocksize(fd);
    size_t bufsize = 8 * base;

    char *buf = malloc(bufsize);
    if (!buf) {
        perror("malloc");
        close(fd);
        return 1;
    }

    ssize_t nr;
    while ((nr = read(fd, buf, bufsize)) > 0) {
        char *outp = buf;
        while (nr > 0) {
            ssize_t nw = write(STDOUT_FILENO, outp, (size_t)nr);
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
