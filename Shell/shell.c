#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <sys/types.h>
#include <pwd.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define MAX_INPUT_LEN 1024
#define MAX_NUM_ARGS 128
#define MAX_PENDING_CONN 5

void setup_server(int port, int *server_socket);
void handle_client(int client_socket);
void send_prompt(int client_socket);
void process_command(char **args, int num_args, int client_socket);
int parse_line(char *input, char **args);
void execute_cmd(char **args, int left, int right, int client_socket);
void get_info(char *info_user, char *info_path);

const char *CMD_EXIT = "exit";
const char *CMD_CD = "cd";
const char *CMD_PIPE = "|";

void setup_server(int port, int *server_socket) {
    struct sockaddr_in server_addr;

    *server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (*server_socket < 0) {
        perror("socket");
        exit(EXIT_FAILURE);
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(port);

    if (bind(*server_socket, (struct sockaddr *)&server_addr, sizeof(server_addr))) {
        perror("bind");
        exit(EXIT_FAILURE);
    }

    if (listen(*server_socket, MAX_PENDING_CONN)) {
        perror("listen");
        exit(EXIT_FAILURE);
    }
}

void handle_client(int client_socket) {
    char input[MAX_INPUT_LEN];
    char *args[MAX_NUM_ARGS];
    ssize_t bytes_read;

    while (1) {
        send_prompt(client_socket);

        bytes_read = read(client_socket, input, sizeof(input) - 1);
        if (bytes_read <= 0)
            break;
            
        input[bytes_read] = '\0';
        printf("received command: %s", input);

        int num_args = parse_line(input, args);
        if (args[0] == NULL) {
            continue;
        }

        process_command(args, num_args, client_socket);
        
        if (strcmp(args[0], CMD_EXIT) == 0) {
            break;
        }
    }
}

void send_prompt(int client_socket) {
    char info_user[MAX_INPUT_LEN] = {0};
    char info_path[MAX_INPUT_LEN] = {0};
    get_info(info_user, info_path);
    
    char prompt[MAX_INPUT_LEN];
    snprintf(prompt, sizeof(prompt), "\033[1;32m%s\033[0m\033[1m:\033[0m\033[1;36m%s\033[0m> ", 
             info_user, info_path);
    write(client_socket, prompt, strlen(prompt));
}

void process_command(char **args, int num_args, int client_socket) {
    if (strcmp(args[0], CMD_EXIT) == 0) return;

    if (strcmp(args[0], CMD_CD) == 0) {
        if (num_args < 2) {
            write(client_socket, "Usage: cd <directory>\n", 22);
            return;
        }
        int ret = chdir(args[1]);
        if (ret) {
            perror("cd");
            write(client_socket, "Error: Unable to change directory\n", 34);
        }
    }
    else {
        execute_cmd(args, 0, num_args, client_socket);
    }
}

int parse_line(char *input, char **args) {
    char *token = strtok(input, " \n\r");
    int i = 0;
    while (token != NULL && i < MAX_NUM_ARGS - 1) {
        args[i++] = token;
        token = strtok(NULL, " \n\r");
    }
    args[i] = NULL;
    return i;
}

void execute_cmd(char **args, int left, int right, int client_socket) {
    int pipe_index = -1;
    for (int i = left; i < right; ++i) {
        if (strcmp(args[i], CMD_PIPE) == 0) {
            pipe_index = i;
            break;
        }
    }
    
    if (pipe_index == -1) {
        pid_t pid = fork();
        if (pid == -1) {
            perror("fork");
            return;
        }
        else if (pid == 0) {
            dup2(client_socket, STDOUT_FILENO);
            dup2(client_socket, STDERR_FILENO);
            execvp(args[left], &args[left]);
            perror("execvp");
            exit(EXIT_FAILURE);
        }
        else {
            wait(NULL);
            return;
        }
    }
    else if (pipe_index + 1 == right) {
        write(client_socket, "Wrong pipe parameters.\n", 23);
        return;
    }

    int fds[2];
    if (pipe(fds) == -1) {
        perror("pipe");
        return;
    }
    
    pid_t pid = fork();
    if (pid == -1) {
        perror("fork");
        return;
    }
    else if (pid == 0) {
        close(fds[0]);
        dup2(fds[1], STDOUT_FILENO);
        close(fds[1]);

        args[pipe_index] = NULL;
        execvp(args[left], &args[left]);
        perror("execvp");
        exit(EXIT_FAILURE);
    }
    else {
        wait(NULL);

        pid_t pid2 = fork();
        if (pid2 == -1) {
            perror("fork");
            return;
        }
        else if (pid2 == 0) {
            close(fds[1]);
            dup2(fds[0], STDIN_FILENO);
            close(fds[0]);
            execute_cmd(args, pipe_index + 1, right, client_socket);
            exit(EXIT_FAILURE);
        }
        else {
            close(fds[0]);
            close(fds[1]);
            wait(NULL);
            return;
        }
    }
}

void get_info(char *info_user, char *info_path) {
    char tmp_buf[MAX_INPUT_LEN] = {0};

    if (info_user == NULL || info_path == NULL) {
        return;
    }

    struct passwd *pwd = getpwuid(getuid());
    if (pwd) {
        strncpy(info_user, pwd->pw_name, MAX_INPUT_LEN - 1);
        strncat(info_user, "@", 1);
    }

    if (gethostname(tmp_buf, MAX_INPUT_LEN) == 0) {
        strncat(info_user, tmp_buf, MAX_INPUT_LEN - strlen(info_user) - 1);
    }

    if (getcwd(tmp_buf, MAX_INPUT_LEN)) {
        strncpy(info_path, tmp_buf, MAX_INPUT_LEN - 1);
    }
}

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s <port>\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    int port = atoi(argv[1]);
    int server_socket;
    
    setup_server(port, &server_socket);

    while (1) {
        struct sockaddr_in client_addr;
        socklen_t client_addr_len = sizeof(client_addr);
        int client_socket = accept(server_socket, 
                                 (struct sockaddr *)&client_addr, 
                                 &client_addr_len);
        
        if (client_socket < 0) {
            perror("accept");
            continue;
        }

        pid_t pid = fork();

        if (pid == -1) {
            perror("fork");
            close(client_socket);
        }
        else if (pid == 0) {
            close(server_socket);
            handle_client(client_socket);
            close(client_socket);
            exit(EXIT_SUCCESS);
        }
        else {
            close(client_socket);
        }
    }

    return EXIT_SUCCESS;
}