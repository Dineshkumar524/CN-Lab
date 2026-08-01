#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>

#define PORT 8080
#define BUFFER_SIZE 1024

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    char reg_no[BUFFER_SIZE];
    char buffer[BUFFER_SIZE];

    // Create Socket
    sock = socket(AF_INET, SOCK_STREAM, 0);

    if (sock < 0)
    {
        printf("Socket creation failed\n");
        return -1;
    }

    // Server Address
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    // Convert IP Address
    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        printf("Invalid Address\n");
        return -1;
    }

    // Connect to Server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        printf("Connection Failed\n");
        return -1;
    }

    printf("Connected to Server...\n");

    while (1)
    {
        printf("\nEnter Registration Number (or type 'bye' to exit): ");
        fgets(reg_no, BUFFER_SIZE, stdin);

        // Remove newline
        reg_no[strcspn(reg_no, "\n")] = 0;

        // Send data to server
        send(sock, reg_no, strlen(reg_no), 0);

        // Exit condition
        if (strcmp(reg_no, "bye") == 0)
        {
            printf("Connection Closed.\n");
            break;
        }

        memset(buffer, 0, BUFFER_SIZE);

        // Receive response
        read(sock, buffer, BUFFER_SIZE);

        printf("Server Response: %s\n", buffer);
    }

    close(sock);

    return 0;
}