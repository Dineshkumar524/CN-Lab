#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/socket.h>

#define PORT 8080
#define SIZE 4096

struct Query {
    int student_id;
    char data[SIZE];
};

int main() {
    int sock;
    struct sockaddr_in server_addr;
    struct Query q;
    char response[SIZE];

    FILE *fp = fopen("data.txt", "r");

    if (fp == NULL) {
        perror("Cannot open data.txt");
        return 1;
    }

    memset(&q, 0, sizeof(q));

    size_t used = fread(q.data, 1, SIZE - 1, fp);
    q.data[used] = '\0';
    fclose(fp);

    printf("Enter Student ID to search: ");
    if (scanf("%d", &q.student_id) != 1) {
        printf("Invalid Student ID\n");
        return 1;
    }

    sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("Socket error");
        return 1;
    }

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);

    // Server running on the same computer
    if (inet_pton(AF_INET, "127.0.0.1",
                  &server_addr.sin_addr) != 1) {
        printf("Invalid server address\n");
        close(sock);
        return 1;
    }

    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0) {
        perror("Connection error");
        close(sock);
        return 1;
    }

    size_t sent = 0;
    while (sent < sizeof(q)) {
        ssize_t n = write(sock, (char *)&q + sent,
                          sizeof(q) - sent);
        if (n <= 0) {
            perror("Send error");
            close(sock);
            return 1;
        }
        sent += n;
    }

    size_t received = 0;
    while (received < sizeof(response)) {
        ssize_t n = read(sock, response + received,
                         sizeof(response) - received);
        if (n <= 0)
            break;

        received += n;

        if (memchr(response, '\0', received) != NULL)
            break;
    }

    if (received > 0) {
        response[SIZE - 1] = '\0';
        printf("Server Response : %s\n", response);
    } else {
        printf("No response from server\n");
    }

    close(sock);
    return 0;
}
