#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>
#include <fcntl.h>

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <source> <destination>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

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

        int src_fd = open(argv[1], O_RDONLY);
        if (src_fd == -1) {
            perror("open source");
            exit(EXIT_FAILURE);
        }

        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(src_fd, buffer, sizeof(buffer))) > 0) {
            if (write(pipefd[1], buffer, bytes_read) == -1) {
                perror("write to pipe");
                exit(EXIT_FAILURE);
            }
        }

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

        int dest_fd = open(argv[2], O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (dest_fd == -1) {
            perror("open destination");
            exit(EXIT_FAILURE);
        }

        char buffer[4096];
        ssize_t bytes_read;
        while ((bytes_read = read(pipefd[0], buffer, sizeof(buffer))) > 0) {
            if (write(dest_fd, buffer, bytes_read) == -1) {
                perror("write to file");
                exit(EXIT_FAILURE);
            }
        }

        close(dest_fd);
        close(pipefd[0]);
        exit(EXIT_SUCCESS);
    }

    close(pipefd[0]);
    close(pipefd[1]);
    waitpid(reader_pid, NULL, 0);
    waitpid(writer_pid, NULL, 0);
    printf("PipeCopy completed.\n");
    return 0;
} 