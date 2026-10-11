#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <ctype.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 30906
#define SIZE 2048
#define ROLL "25157142906"

int main() {
    int sockfd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_len;
    char buffer[SIZE], response[SIZE];

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(sockfd, (struct sockaddr *)&server_addr,
             sizeof(server_addr)) < 0) {
        perror("Bind error");
        close(sockfd);
        return 1;
    }

    printf("Server running on port %d\n", PORT);

    while (1) {
        client_len = sizeof(client_addr);

        int n = recvfrom(sockfd, buffer, SIZE - 1, 0,
                         (struct sockaddr *)&client_addr,
                         &client_len);

        if (n < 0) {
            perror("Receive error");
            continue;
        }

        buffer[n] = '\0';

        printf("\nClient IP: %s, Port: %d\n",
               inet_ntoa(client_addr.sin_addr),
               ntohs(client_addr.sin_port));

        char *separator = strchr(buffer, '#');

        if (separator == NULL) {
            strcpy(response, "ERROR: Unauthorized client");
        } else {
            *separator = '\0';
            char *roll = buffer;
            char *message = separator + 1;

            if (strcmp(roll, ROLL) != 0) {
                strcpy(response, "ERROR: Unauthorized client");
            } else if (strcmp(message, "EXIT") == 0) {
                strcpy(response, "Server shutting down");

                sendto(sockfd, response, strlen(response), 0,
                       (struct sockaddr *)&client_addr,
                       client_len);

                printf("Received message: EXIT\n");
                printf("Sent response: %s\n", response);
                break;
            } else {
                printf("Received message: %s\n", message);

                char *bar = strchr(message, '|');

                if (bar == NULL) {
                    strcpy(response, "ERROR: Invalid command");
                } else {
                    *bar = '\0';
                    char *command = message;
                    char *text = bar + 1;

                    if (strcmp(command, "UP") == 0) {
                        for (int i = 0; text[i]; i++)
                            text[i] = toupper(
                                (unsigned char)text[i]);

                        snprintf(response, SIZE, "%s", text);

                    } else if (strcmp(command, "LOW") == 0) {
                        for (int i = 0; text[i]; i++)
                            text[i] = tolower(
                                (unsigned char)text[i]);

                        snprintf(response, SIZE, "%s", text);

                    } else if (strcmp(command, "REV") == 0) {
                        int len = strlen(text);

                        for (int i = 0; i < len / 2; i++) {
                            char temp = text[i];
                            text[i] = text[len - 1 - i];
                            text[len - 1 - i] = temp;
                        }

                        snprintf(response, SIZE, "%s", text);

                    } else {
                        strcpy(response, "ERROR: Invalid command");
                    }
                }
            }
        }

        sendto(sockfd, response, strlen(response), 0,
               (struct sockaddr *)&client_addr, client_len);

        printf("Sent response: %s\n", response);
    }

    close(sockfd);
    printf("Server stopped.\n");
    return 0;
}
