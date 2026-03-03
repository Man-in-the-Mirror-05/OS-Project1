#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/time.h>

void copy(int src_fd, int dest_fd) {
    size_t buf_size = atoi(getenv("BUFFER_SIZE") ?: "4096");
    char *buffer = malloc(buf_size);

    ssize_t bytes_read;
    while ((bytes_read = read(src_fd, buffer, buf_size)) > 0) {
        if (write(dest_fd, buffer, bytes_read) != bytes_read) {
            perror("write");
            free(buffer);
            exit(EXIT_FAILURE);
        }
    }
    
    free(buffer);

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

    struct timeval start, end;
    gettimeofday(&start, NULL);

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

    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds*1e-6;

    close(src_fd);
    close(dest_fd);
    
    printf("MyCopy completed in %.6f seconds.\n", elapsed);
    return 0;
}