// mycat6.c
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <sys/stat.h>
#include <stdint.h>

// 获取文件系统推荐的 I/O 块大小或页大小
static size_t io_blocksize(int fd) {
    struct stat st;
    if (fstat(fd, &st) == 0 && st.st_blksize > 0) {
        return (size_t)st.st_blksize;
    }
    long ps = sysconf(_SC_PAGESIZE);
    return (ps > 0) ? (size_t)ps : 4096;
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *msg = "Usage: mycat6 <file>\n";
        write(STDERR_FILENO, msg, 17);
        return 1;
    }

    const char *path = argv[1];
    int fd = open(path, O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }

    // 提示内核按顺序读取并预读
    if (posix_fadvise(fd, 0, 0, POSIX_FADV_SEQUENTIAL) != 0) {
        perror("posix_fadvise");
        // 虽然失败，但继续执行
    }

    // 使用 mycat5 最佳经验，8×基础块大小
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
    if (nr < 0) perror("read");

    free(buf);
    close(fd);
    return (nr < 0) ? 1 : 0;
}
