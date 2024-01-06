#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include "packet.h"

int main(int argc, char *argv[]) {
    /*
     * Validates command line arguments.
     *
     * Checks that exactly 4 arguments were passed:
     * - argv[0]: Program name
     * - argv[1]: IPv6 address
     * - argv[2]: Port number
     * - argv[3]: sNumber
     *
     * Prints usage message and returns error if validation fails.
     */
    if (argc != 4)
    {
        printf("Usage: %s [IPv6_Address] [Port] [sNumber]\n", argv[0]);
        return 1;
    }

    char *ipv6_addr = argv[1];
    
   /*
     * Parses port number from command line argument.
     * Validates port is 50000, exits with error if not.
     */
    int port = atoi(argv[2]);
    if (port != 50000)
    {
        perror("[!] Please use port 50.000. Exiting.");
        exit(EXIT_FAILURE);
    }

    /*
     * Parses sNumber argument from command line.
     * Validates that sNumber is a 5 digit integer between 10000 and 99999.
     * Prints error and exits if invalid.
     */
    int sNumber = atoi(argv[3]); 
    if (sNumber < 10000 || sNumber > 99999)
    { 
        printf("[!] sNumber is invalid\n");
        exit(EXIT_FAILURE);
    }

    /*
     * Creates an IPv6 socket of type SOCK_STREAM and checks for errors.
     * Exits if error occurs.
     */
    int sockfd = socket(AF_INET6, SOCK_STREAM, 0);
    if (!sockfd)
    {
        perror("[!] Socket creation failed");
        exit(EXIT_FAILURE);    
    }


    /*
     * Initializes a struct sockaddr_in6 named server_addr to store the server
     * address information. This will be used later when connecting to the server.
     * 
     * - memset initializes the struct to 0.
     * - sin6_family is set to AF_INET6 for IPv6.
     * - sin6_port is set to the server port converted to network byte order.
     * - inet_pton converts the IPv6 address string to binary and stores it in sin6_addr.
    */
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    server_addr.sin6_port = htons(port);
    inet_pton(AF_INET6, ipv6_addr, &server_addr.sin6_addr);

    /*
     * Attempts to connect the socket to the server.
     * The connect() system call is used to establish a connection to the
     * server address specified in server_addr.
     * If the connection fails, an error is printed and the program exits.
     */
    if (connect(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("[!] Connection failed. Exiting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }
    printf("[+]-Socket created.\n");
    printf("[+]-reciving from %s on port %d\n", ipv6_addr, port);

    /* 
     * Declares data structures for sending/receiving packets, monitoring socket descriptors, and setting a timeout 
     */
    struct packet data;
    fd_set readfds;
    struct timeval tv;

    /*
     * Main client-server messaging loop:
     *
     * - Prompts user to enter a message
     * - Sends message to server
     * - Waits up to 60 seconds for response from server
     * - Prints received message if available, else prints timeout
     * - Exits loop if user enters "exit"
     */
    while (1)
    {
        printf("s%d > ", sNumber);
        fgets(data.text, MAX_TEXT_LEN, stdin);
        data.text[strcspn(data.text, "\n")] = '\0';
        data.sNumber = sNumber;
        if (strcmp(data.text, "exit") == 0)
        {
            send(sockfd, &data, sizeof(data), 0);
            break;
        }
        send(sockfd, &data, sizeof(data), 0);

        FD_ZERO(&readfds);
        FD_SET(sockfd, &readfds);

        tv.tv_sec = 60;
        tv.tv_usec = 0;

        select(sockfd + 1, &readfds, NULL, NULL, &tv);

        if (FD_ISSET(sockfd, &readfds))
        {
            recv(sockfd, &data, sizeof(data), 0);
            printf("s%d > %s\n", data.sNumber, data.text);
        }
        else
        {
            printf("[!] No response from the server.\n");
        }
    }

    /*
     * Closes the socket file descriptor. 
     */
    close(sockfd);
    return 0;
}
