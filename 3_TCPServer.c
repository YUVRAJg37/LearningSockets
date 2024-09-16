#ifndef _WIN32_WINNT
#define _WIN32_WINNT 0x0600
#endif

#include <WinSock2.h>
#include <WS2tcpip.h>

#include <stdio.h>
#include <ctype.h>

int main()
{
    WSADATA d;
    if (WSAStartup(MAKEWORD(2, 2), &d))
    {
        printf("Startup Failed.\n");
        return 1;
    }

    struct addrinfo hints, *peer_address;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    getaddrinfo(0, "8080", &hints, &peer_address);

    printf("Creating Socket.\n");

    SOCKET socket_listen = socket(peer_address->ai_family, peer_address->ai_socktype, peer_address->ai_protocol);

    if (socket_listen == SOCKET_ERROR)
    {
        printf("Error Creating Socket.\n");
        return 1;
    }

    printf("Binding Address.\n");

    if (bind(socket_listen, peer_address->ai_addr, peer_address->ai_addrlen))
    {
        printf("Binding Failed.\n");
        return 1;
    }

    freeaddrinfo(peer_address);

    printf("Listening...\n");

    if (listen(socket_listen, 2) < 0)
    {
        printf("Disconneted...\n");
        return 1;
    }

    fd_set master;
    FD_ZERO(&master);
    FD_SET(socket_listen, &master);
    SOCKET max_socket = socket_listen;

    printf("Waiting For Connection....\n");

    while (1)
    {
        fd_set reads;
        reads = master;

        if (select(max_socket + 1, &reads, 0, 0, 0) < 0)
        {
            printf("Select() failed.\n");
            return 1;
        }

        for (SOCKET i = 1; i <= max_socket; i++)
        {
            if (FD_ISSET(i, &reads))
            {
                if (i == socket_listen)
                {
                    struct addrinfo client_address;
                    socklen_t client_address_len = sizeof(client_address);
                    SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_address_len);

                    if (socket_client == SOCKET_ERROR)
                    {
                        printf("Error Getting Client.\n");
                        return 1;
                    }

                    FD_SET(socket_client, &master);
                    if (socket_client > max_socket)
                        max_socket = socket_client;

                    char address_buffer[100];
                    getnameinfo((struct sockaddr *)&client_address, client_address_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
                    printf("New Connection: %s.\n", address_buffer);
                }
                else
                {
                    char read[1024];
                    int bytes_received = recv(i, read, 1024, 0);
                    if (bytes_received < 1)
                    {
                        FD_CLR(i, &master);
                        closesocket(i);
                        continue;
                    }

                    for (SOCKET j = 1; j <= max_socket; j++)
                    {
                        if (FD_ISSET(j, &master))
                        {
                            if (j == socket_listen || j == i)
                            {
                                continue;
                            }
                            else
                            {
                                send(j, read, bytes_received, 0);
                            }
                        }
                    }
                }
            }
        }
    }

    printf("Closing Listening to Socket.\n");
    closesocket(socket_listen);

    WSACleanup();

    printf("Finished.\n");
    return 0;
}