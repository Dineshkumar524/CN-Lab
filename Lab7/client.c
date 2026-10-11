#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 30906
#define SIZE 2048

int main(int argc, char *argv[]) {
    int sockfd;
    struct sockaddr_in server_addr;
    socklen_t server_len = sizeof(server_addr);
    char input[SIZE], message[SIZE], response[SIZE];

    if (argc != 3) {
        printf("Usage: %s <server_ip> <roll>\n", argv[0]);
        return 1;
    }

    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, argv[1],
                  &server_addr.sin_addr) != 1) {
        printf("Invalid server IP address\n");
        close(sockfd);
        return 1;
    }

    printf("UDP Client started.\n");
    printf("Commands: UP|text, LOW|text, REV|text, EXIT\n");

    while (1) {
        printf("\nEnter command: ");
        fflush(stdout);

        if (fgets(input, sizeof(input), stdin) == NULL)
            strcpy(input, "EXIT");

        input[strcspn(input, "\n")] = '\0';

        snprintf(message, sizeof(message),
                 "%s#%s", argv[2], input);

        if (sendto(sockfd, message, strlen(message), 0,
                   (struct sockaddr *)&server_addr,
                   server_len) < 0) {
            perror("Send error");
            break;
        }

        struct timeval timeout = {5, 0};
        setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO,
                   &timeout, sizeof(timeout));

        int n = recvfrom(sockfd, response, SIZE - 1, 0,
                         NULL, NULL);

        if (n < 0) {
            perror("Receive error");
            break;
        }

        response[n] = '\0';
        printf("Server Response: %s\n", response);

        if (strcmp(input, "EXIT") == 0)
            break;
    }

    close(sockfd);
    return 0;
}
