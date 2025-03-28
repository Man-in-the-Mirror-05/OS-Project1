#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

void copy(int src_fd, int dest_fd) {
    char buffer[1024];
    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
        write(dest_fd, buffer, bytes_read);
    }
    if (bytes_read == -1) {
        perror("read");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int src_fd = open(argv[1], O_RDONLY);
    if (src_fd == -1) {
        perror("open source");
        exit(EXIT_FAILURE);
    }

    int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
    if (dest_fd == -1) {
        perror("open destination");
        close(src_fd);
        exit(EXIT_FAILURE);
    }

    copy(src_fd, dest_fd);

    close(src_fd);
    close(dest_fd);
    printf("MyCopy completed.\n");
    return 0;
}