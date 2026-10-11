
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PORT 8080
#define SIZE 4096

struct Query {
    int student_id;
    char data[SIZE];
};

int main() {
    int server_fd, client_fd;
    struct sockaddr_in server_addr;

    signal(SIGCHLD, SIG_IGN);

    server_fd = socket(AF_INET, SOCK_STREAM, 0);
    if (server_fd < 0) {
        perror("Socket error");
        return 1;
    }

    int opt = 1;
    setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR,
               &opt, sizeof(opt));

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Bind error");
        close(server_fd);
        return 1;
    }

    if (listen(server_fd, 5) < 0) {
        perror("Listen error");
        close(server_fd);
        return 1;
    }

    printf("Server is running on port %d...\n", PORT);

    while (1) {
        client_fd = accept(server_fd, NULL, NULL);

        if (client_fd < 0) {
            perror("Accept error");
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            // Child process
            close(server_fd);

            struct Query q;
            char response[SIZE];
            memset(&q, 0, sizeof(q));

            size_t received = 0;
            while (received < sizeof(q)) {
                ssize_t n = read(client_fd,
                                 (char *)&q + received,
                                 sizeof(q) - received);
                if (n <= 0)
                    break;
                received += n;
            }

            if (received == sizeof(q)) {
                q.data[SIZE - 1] = '\0';
                char *line = strtok(q.data, "\n");
                int found = 0;

                while (line != NULL) {
                    int id;
                    char name[100], department[100];

                    if (sscanf(line, "%d %99s %99s",
                               &id, name, department) == 3
                        && id == q.student_id) {
                        snprintf(response, sizeof(response),
                                 "Record Found : %s", line);
                        found = 1;
                        break;
                    }

                    line = strtok(NULL, "\n");
                }

                if (!found) {
                    snprintf(response, sizeof(response),
                             "Record Not Found");
                }

                write(client_fd, response, strlen(response) + 1);
            }

            close(client_fd);
            exit(0);
        } else if (pid > 0) {
            // Parent process
            close(client_fd);
        } else {
            perror("Fork error");
            close(client_fd);
        }
    }

    close(server_fd);
    return 0;
}
