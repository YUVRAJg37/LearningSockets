#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <stdio.h>
#include <conio.h>

int main(int argc, char *argv[])
{
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        fprintf(stderr, "WSA Startup failed.\n");
        return 1;
    }

    // arg contains the name of program also, so first arg is always its name
    if (argc < 3)
    {
        fprintf(stderr, "usage: tcp_client hostname port\n");
        return 1;
    }

    printf("Configuring Remote Address.\n");
    struct addrinfo hints, *peer_address;
    memset(&hints, 0, sizeof(hints));
    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
    {
        fprintf(stderr, "Get Addr Info Failed.\n");
        return 1;
    }

    printf("Remote Address is: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen,
                address_buffer, sizeof(address_buffer),
                service_buffer, sizeof(service_buffer), NI_NUMERICHOST);

    printf("%s %s\n", address_buffer, service_buffer);

    printf("Creating Socket.\n");
    SOCKET peer_socket;
    peer_socket = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);

    if (peer_socket == INVALID_SOCKET)
    {
        fprintf(stderr, "Socket creation failed.\n");
        return 1;
    }

    // Connect used to connect to remote address.
    if (connect(peer_socket, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        fprintf(stderr, "Connection failed.\n");
        return 1;
    }

    freeaddrinfo(peer_address);

    printf("Connected.\n");
    printf("To Send data, enter text.\n");

    while (1)
    {
        // placing our socket inside a set used by various functions to test our sockets.
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(peer_socket, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        // Return if timeout or any socket is ready for read.
        if (select(peer_socket + 1, &reads, 0, 0, &timeout) < 0)
        {
            fprintf(stderr, "Select() failed.\n");
            return 1;
        }

        // If socket is ready to read
        if (FD_ISSET(peer_socket, &reads))
        {
            char read[4096];
            int bytes_received = recv(peer_socket, read, 4096, 0);

            if (bytes_received < 1)
            {
                fprintf(stderr, "Connection Closed.\n");
                break;
            }

            printf("Received (%d bytes) %.*s\n", bytes_received, bytes_received, read);
        }

        // Wait until we are typing in console.
        if (_kbhit())
        {
            char read[4096];
            // Reading from console.
            if (!fgets(read, 4096, stdin))
                break;
            printf("Sending: %s", read);
            int bytes_sent = send(peer_socket, read, strlen(read), 0);
            printf("Send %d bytes\n", bytes_sent);
        }
    }

    printf("Closing Socket.\n");

    closesocket(peer_socket);
    WSACleanup();

    printf("Finished.\n");

    return 0;
}