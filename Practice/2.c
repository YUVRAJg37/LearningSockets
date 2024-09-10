#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <winsock2.h>
#include <WS2tcpip.h>

#include <stdio.h>
#include <conio.h>

int main(int argc, char *argv[])
{
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        printf("Startup Failed.\n");
        return 1;
    }

    if (argc < 3)
    {
        printf("Not enough arguments.\n");
        return 1;
    }
    printf("Resolving Address.\n");
    struct addrinfo hints, *peer_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;

    if (getaddrinfo(argv[1], argv[2], &hints, &peer_address))
    {
        printf("Getting Address Failed");
        return 1;
    }

    printf("Remote Address: ");
    char address_buffer[100];
    char service_buffer[100];
    getnameinfo(peer_address->ai_addr, peer_address->ai_addrlen, address_buffer, sizeof(address_buffer), service_buffer, sizeof(service_buffer), NI_NUMERICHOST);
    printf("%s %s\n", address_buffer, service_buffer);

    printf("Creating Socket.\n");
    SOCKET socket_peer;
    socket_peer = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);
    if (socket_peer == SOCKET_ERROR)
    {
        printf("Invalid Socket.\n");
        return 1;
    }

    printf("Connecting Socket.\n");

    if (connect(socket_peer, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        printf("Connection Failed.\n");
        return 1;
    }

    freeaddrinfo(peer_address);

    printf("Connected.\n");

    while (1)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
        {
            printf("Connection Closed.\n");
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads))
        {
            char read[4096];
            int bytes_received = recv(socket_peer, read, 4096, 0);
            printf("Recevied %d bytes\n", bytes_received);
            printf("Message : %.*s\n", bytes_received, read);
        }

        if (_kbhit())
        {
            char read[4096];
            if (!fgets(read, 4096, stdin))
            {
                break;
            }
            int bytes_sent = send(socket_peer, read, strlen(read), 0);
            printf("%d bytes sent.\n", bytes_sent);
        }
    }

    printf("Finished");
    closesocket(socket_peer);
    WSACleanup();

    return 1;
}