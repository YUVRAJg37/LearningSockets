#ifndef _WIN32_WINNT
// WIN NEW TECHNOLOGY MINIMUM VERSION SUPPORT
// https://learn.microsoft.com/en-us/cpp/porting/modifying-winver-and-win32-winnt?view=msvc-170
#define _WIN32_WINNT 0x0600
#endif

#if !defined(IPV6_V6ONLY)
#define IPV6_V6ONLY 27
#endif

#include <winsock2.h>
#include <WS2tcpip.h>

#include <stdio.h>
#include <string.h>
#include <time.h>

// 1. Initialization of WSA.
// 2. Getting Address Info of the connection at specific PORT.
// 3. Creating a Socket.
// 4. Binding that Socket to Address.
// 5. Starts Listening to Connections.
// 6. Accepting Client Connection.
// 7. Optional. Printing the client address.
// 8. Sending Response.
// 9. Closing Connections, WSA and cleanup.

int main()
{

    // Initializing Windows Socket API
    WSADATA d;
    // MAKEWORD will create a WORD 32 byte.
    // 2.2 version of WSA.
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "Failed to initialize.\n");
        return 1;
    }

    printf("Ready to use socket API.\n");
    printf("Configuring local Address.\n");

    struct addrinfo hints;
    struct addrinfo *bind_address;
    memset(&hints, 0, sizeof(hints));

    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    // Enabled WildCard Address.
    // i.e., listen for incoming connections on all available network interfaces of the host machine
    // If your machine has multiple network interfaces (e.g., a loopback address like 127.0.0.1,
    // a private IP like 192.168.1.10, and a public IP assigned by your ISP), binding to the wildcard
    // address means that your server will accept connections coming through any of those interfaces.
    hints.ai_flags = AI_PASSIVE;

    // Address info of the connection at localhost:8080
    getaddrinfo(0, "8080", &hints, &bind_address);

    printf("Creating Socket.\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family, bind_address->ai_socktype, bind_address->ai_protocol);

    if (socket_listen == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket failed to initliaze. (%d)\n", WSAGetLastError());
        return 1;
    }

    // Clearing option for IPV6_ONLY flag.
    int option = 0;
    if (setsockopt(socket_listen, IPPROTO_IPV6, IPV6_V6ONLY, (void *)&option, sizeof(option)))
    {
        fprintf(stderr, "Error setting socket options %d", WSAGetLastError());
        return 1;
    }

    printf("Binding socket to local address.\n");
    if (bind(socket_listen, bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "Socket binding failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    freeaddrinfo(bind_address);

    printf("Listening...\n");

    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "Listening failed. (%d)\n", WSAGetLastError());
        return 1;
    }

    printf("Waiting for connection...\n");

    struct sockaddr_storage client_address;
    socklen_t client_len = sizeof(client_address);
    SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);

    if (socket_client == INVALID_SOCKET)
    {
        fprintf(stderr, "Client listening failed. (%d)", WSAGetLastError());
        return 1;
    }

    printf("Client is Connected...\n");

    char address_buffer[100];
    getnameinfo((struct sockaddr *)&client_address, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
    printf("%s\n", address_buffer);

    printf("Reading request...\n");
    char request[1024];
    int bytes_received = recv(socket_client, request, 1024, 0);
    printf("Received %d bytes.\n", bytes_received);
    // printf("Bytes Received %.*s", bytes_received, request);

    printf("Sending Response...\n");
    const char *response = "HTTP/1.1 200 OK\r\n"
                           "Connection: close\r\n"
                           "Content-Type: text/plain\r\n\r\n"
                           "Local time is: ";
    int bytes_sent = send(socket_client, response, strlen(response), 0);
    printf("Send %d bytes of %d\n", bytes_sent, (int)strlen(response));

    time_t timer;
    time(&timer);
    char *time_message = ctime(&timer);
    bytes_sent = send(socket_client, time_message, strlen(time_message), 0);
    printf("Send %d bytes of %d\n", bytes_sent, (int)strlen(time_message));

    printf("Closing connection...\n");
    closesocket(socket_client);

    printf("Closing listening socket...\n");
    closesocket(socket_listen);

    WSACleanup();
    printf("Finished.\n");

    return 0;
}