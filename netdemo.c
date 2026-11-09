/*
 * netdemo.c - small cross-platform example for PE/ELF analysis
 *
 * Performs:
 *   1. Read an environment variable.
 *   2. Resolve a hostname with getaddrinfo().
 *   3. Try a TCP connection to port 443.
 *   4. Close the socket and release the address list.
 *
 * IMPORTANT: This is TCP only. It does NOT perform a TLS handshake
 * and does NOT send HTTP/HTTPS data.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifdef _WIN32
    /* winsock2.h must be included before windows.h, if windows.h is used. */
    #include <winsock2.h>
    #include <ws2tcpip.h>

    typedef SOCKET socket_t;
    #define CLOSE_SOCKET(s) closesocket(s)
    #define SOCKET_IS_INVALID(s) ((s) == INVALID_SOCKET)
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netdb.h>
    #include <unistd.h>

    typedef int socket_t;
    #define CLOSE_SOCKET(s) close(s)
    #define SOCKET_IS_INVALID(s) ((s) < 0)
#endif

int main(int argc, char *argv[])
{
    const char *hostname = "www.example.com";
    const char *username = NULL;
    const char *port = "443";

    struct addrinfo hints;
    struct addrinfo *addresses = NULL;
    struct addrinfo *entry;

    int connected = 0;
    int lookup_status;

#ifdef _WIN32
    WSADATA wsa_data;

    if (WSAStartup(MAKEWORD(2, 2), &wsa_data) != 0) {
        fprintf(stderr, "WSAStartup failed\n");
        return 1;
    }
    username = getenv("USERNAME");
#else
    username = getenv("USER");
#endif

    /* 1. Read and print an environment variable. */
    printf("User: %s\n", username != NULL ? username : "(not set)");

    /* 2. Optional hostname from the command line. */
    if (argc > 1) {
        hostname = argv[1];
    }

    printf("Target: %s:%s\n", hostname, port);

    /* 3. Resolve the hostname for a TCP connection. */
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_UNSPEC;       /* IPv4 or IPv6 */
    hints.ai_socktype = SOCK_STREAM;   /* TCP */
    hints.ai_protocol = IPPROTO_TCP;

    lookup_status = getaddrinfo(hostname, port, &hints, &addresses);
    if (lookup_status != 0) {
#ifdef _WIN32
        fprintf(stderr, "getaddrinfo failed: %d\n", lookup_status);
        WSACleanup();
#else
        fprintf(stderr, "getaddrinfo failed: %s\n",
                gai_strerror(lookup_status));
#endif
        return 1;
    }

    puts("Name resolution successful.");

    /* 4. Try each returned address until one TCP connection succeeds. */
    for (entry = addresses; entry != NULL; entry = entry->ai_next) {
        socket_t sock;

        sock = socket(entry->ai_family,
                      entry->ai_socktype,
                      entry->ai_protocol);

        if (SOCKET_IS_INVALID(sock)) {
            continue;
        }

        puts("Trying TCP connection...");

#ifdef _WIN32
        if (connect(sock, entry->ai_addr, (int)entry->ai_addrlen) == 0) {
#else
        if (connect(sock, entry->ai_addr,
                    (socklen_t)entry->ai_addrlen) == 0) {
#endif
            puts("TCP connection successful.");
            connected = 1;
            CLOSE_SOCKET(sock);
            break;
        }

        puts("TCP connection failed for this address.");
        CLOSE_SOCKET(sock);
    }

    /* 5. Release resources. */
    freeaddrinfo(addresses);

#ifdef _WIN32
    WSACleanup();
#endif

    if (!connected) {
        puts("Could not establish a TCP connection.");
        return 2;
    }

    puts("Done. No TLS handshake or application data was sent.");
    return 0;
}
