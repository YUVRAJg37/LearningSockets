#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <stdio.h>
#include <string.h>

int main()
{
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        printf("WS Initilization Failed.\n");
        return 1;
    }

    printf("Starting...\n");
    struct addrinfo hint, *bind_address;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;

    getaddrinfo(0, "8080", &hint, &bind_address);

    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (socket_listen == INVALID_SOCKET)
    {
        printf("Invalid Socket.\n");
        return 1;
    }

    printf("Binding.\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        printf("Binding Failed.\n");
        return 1;
    }

    freeaddrinfo(bind_address);

    printf("Listening...\n");
    if (listen(socket_listen, 10) < 0)
    {
        printf("Overflow...");
        return 1;
    }

    struct sockaddr_storage client_addr;
    socklen_t client_len = sizeof(client_addr);
    SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_addr, &client_len);

    if (socket_client == INVALID_SOCKET)
    {
        printf("Invalid client.\n");
        return 1;
    }

    char request[1024];

    recv(socket_client, request, 1024, 0);

    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Connection: close\r\n"
                           "Content-Type: text/plain\r\n\r\n"
                           "Local time is: ";
    send(socket_client, response, strlen(response), 0);

    const char *msg = "Hello";
    send(socket_client, msg, strlen(msg), 0);

    closesocket(socket_client);
    closesocket(socket_listen);

    printf("Finished\n");

    WSACleanup();
    return 0;
}