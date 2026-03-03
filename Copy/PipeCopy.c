#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <signal.h>
#include <sys/time.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    struct timeval start, end;
    gettimeofday(&start, NULL);

    int pipefd[2];
    if (pipe(pipefd) == -1) {
        perror("pipe");
        exit(EXIT_FAILURE);
    }

    pid_t reader_pid = fork();
    if (reader_pid == -1) {
        perror("fork reader");
        exit(EXIT_FAILURE);
    }

    if (reader_pid == 0) {
        close(pipefd[0]);

        size_t buf_size = atoi(getenv("BUFFER_SIZE") ?: "4096");
        char *buffer = malloc(buf_size);
        if (!buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        int src_fd = open(argv[1], O_RDONLY);
        if (src_fd == -1) {
            perror("open source");
            free(buffer);
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read;
        while ((bytes_read = read(src_fd, buffer, buf_size)) > 0) {
            if (write(pipefd[1], buffer, bytes_read) == -1) {
                perror("write to pipe");
                free(buffer);
                close(src_fd);
                exit(EXIT_FAILURE);
            }
        }

        free(buffer);
        close(src_fd);
        close(pipefd[1]);
        exit(EXIT_SUCCESS);
    }

    pid_t writer_pid = fork();
    if (writer_pid == -1) {
        perror("fork writer");
        exit(EXIT_FAILURE);
    }

    if (writer_pid == 0) {
        close(pipefd[1]);

        size_t buf_size = atoi(getenv("BUFFER_SIZE") ?: "4096");
        char *buffer = malloc(buf_size);
        if (!buffer) {
            perror("malloc");
            exit(EXIT_FAILURE);
        }

        int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dest_fd == -1) {
            perror("open destination");
            free(buffer);
            exit(EXIT_FAILURE);
        }

        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, buf_size)) > 0) {
            if (write(dest_fd, buffer, bytes_read) == -1) {
                perror("write to file");
                free(buffer);
                close(dest_fd);
                exit(EXIT_FAILURE);
            }
        }

        free(buffer);
        close(dest_fd);
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(reader_pid, NULL, 0);
    waitpid(writer_pid, NULL, 0);
    
    gettimeofday(&end, NULL);
    long seconds = end.tv_sec - start.tv_sec;
    long microseconds = end.tv_usec - start.tv_usec;
    double elapsed = seconds + microseconds*1e-6;
    
    printf("PipeCopy completed in %.6f seconds.\n", elapsed);
    return 0;
}