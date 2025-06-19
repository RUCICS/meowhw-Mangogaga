// mycat1.c
#include <unistd.h>
#include <fcntl.h>
#include <stdlib.h>
#include <stdio.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        const char *msg = "Usage: mycat1 <file>\n";
        write(STDERR_FILENO, msg, 22);
        return 1;
    }
    int fd = open(argv[1], O_RDONLY);
    if (fd < 0) {
        perror("open");
        return 1;
    }
    char c;
    ssize_t n;
    while ((n = read(fd, &c, 1)) > 0) {
        if (write(STDOUT_FILENO, &c, 1) != 1) {
            perror("write");
            close(fd);
            return 1;
        }
    }
    close(fd);
    return 0;
}
