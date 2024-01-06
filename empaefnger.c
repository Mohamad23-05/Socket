#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <sys/types.h>
#include <sys/select.h>
#include <fcntl.h>
#include "packet.h"

#define MAX_CLIENTS 5  // Maximale Anzahl an Clients

/* FCNTL 
void set_nonblock(int socket) {
    int flags;
    flags = fcntl(socket,F_GETFL,0);
    //assert(flags != -1);
    fcntl(socket, F_SETFL, flags | O_NONBLOCK);
}
*/


int main(int argc, char *argv[]) {

    /**
     * stdin_fd: File descriptor for standard input.
     */
    //int stdin_fd = fileno(stdin);

    /*
     * Validates command line arguments.
     * 
     * Checks that exactly 3 arguments were passed:
     * - argv[0]: Program name
     * - argv[1]: Port
     * - argv[2]: sNumber
     * 
     * Prints usage message and exits with failure if validation fails.
     */
    if (argc != 3)
    {
        printf("Usage: %s [Port] [sNumber]\n", argv[0]);
        exit(EXIT_FAILURE);
    }

    /*
     * Parses port number from command line argument.
     * Validates port is 50000, exits with error if not.
     */
    int port = atoi(argv[1]);
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
    int sNumber = atoi(argv[2]); 
    if (sNumber < 10000 || sNumber > 99999)
    { //
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
     * Initializes a sockaddr_in6 struct 'server_addr' that will be used to bind
     * the socket.
     *
     * Sets all bytes to 0 with memset(), sets the address family to IPv6,
     * assigns the loopback address (::1), and sets the port.
     */
    struct sockaddr_in6 server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin6_family = AF_INET6;
    inet_pton(AF_INET6, "::1", &(server_addr.sin6_addr));
    server_addr.sin6_port = htons(port);

    //For in-lab testing with link local
    /* 
     * Initializes a sockaddr_in6 struct 'server_addr' that will be used to bind
     * the socket. Sets all bytes to 0 with memset(). Sets the address family to
     * IPv6, assigns the link-local address fe80::1, and sets the port. 
     */

    // memset(&server_addr, 0, sizeof(server_addr));
    // server_addr.sin6_family = AF_INET6;
    // inet_pton(AF_INET6, "fe80::1", &(server_addr.sin6_addr));
    // server_addr.sin6_port = htons(port);


    /* 
     * Binds the socket to the server address.
     * Checks for errors and exits if binding fails. 
     */
    if (bind(sockfd, (struct sockaddr *)&server_addr, sizeof(server_addr)) == -1)
    {
        perror("[!] Binding failed. Exiting.");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    /* 
     * Listens for incoming client connections on the socket.
     * Specifies a backlog of MAX_CLIENTS pending connections.
     * Exits with failure if listen fails. 
     */
    if (listen(sockfd, MAX_CLIENTS) == -1)
    {
        perror("[!] Listening failed. Exiting");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("[+] Server listening on port %d...\n", port);

    /*
     * Declares variables used for receiving data from clients:
     */
    
     /*
      * data: Struct to store received packet data
      * readfds: File descriptor set for select()
      * tv: Timeout for select()
      */

    struct packet data;
    fd_set readfds;
    struct timeval tv;
    
    /*
     * clientfd[]: Array of client file descriptors
     * client_len: Length of client address
     * client_addr: Address of connected client
     */
    int clientfd[MAX_CLIENTS];
    socklen_t client_len;
    struct sockaddr_in6 client_addr;

    /*
     * max_clients: Maximum number of clients
     * client_count: Current number of connected clients
     */
    int max_clients = MAX_CLIENTS;
    int client_count = 0;

    while (1) {
        /*
         * Clears the file descriptor set readfds and then adds all current client
         * file descriptors to it so that select() will monitor them for
         * incoming data or disconnections. This is done before each call to
         * select() to update the set of descriptors to monitor.
         */
        FD_ZERO(&readfds);
        for (int i = 0; i < client_count; ++i)
        {
            FD_SET(clientfd[i], &readfds);
        }

        /* Adds the server socket file descriptor sockfd to the readfds set so that
         * select() will monitor it for incoming client connection requests. */
        FD_SET(sockfd, &readfds);
        // FD_SET(stdin, &readfds);

        /* 
         * Sets timeout value for select() call.
         * tv.tv_sec is timeout in seconds.
         * tv.tv_usec is timeout in microseconds.
         * This sets timeout to 60 seconds. 
         */
        tv.tv_sec = 60;
        tv.tv_usec = 0;

        /*
         * Checks for activity on the socket file descriptors in readfds, with a timeout of tv.
         * Returns the number of ready descriptors, 0 on timeout, or -1 on error.
         */
        int activity = select(sockfd + client_count + 1, &readfds, NULL, NULL, &tv);

        if (activity < 0)
        {
            perror("[!] Select error. Retrying.");
            continue;
        }

        /*
         * Accepts a new incoming connection on the listening socket sockfd.
         * Stores the new socket descriptor in clientfd array at index client_count.
         * Increments client_count after adding new client.
         * Prints message when new client connects.
         * Continues on failure to accept connection.
         */
        if (FD_ISSET(sockfd, &readfds))
        {
            client_len = sizeof(client_addr);
            clientfd[client_count] = accept(sockfd, (struct sockaddr *)&client_addr, &client_len);
            if (clientfd[client_count] < 0)
            {
                perror("[!] Accept failed. Retrying.");
                continue;
            }
            printf("[+] New client connected\n");
            client_count++;
            // set_nonblock(sockfd);
        }

        
        /*
         * Loops through all connected clients and handles I/O with each one.
         * Checks for incoming data using select() via &readfs and recv(), prints received messages.
         * Allows sending responses back to clients using send().
         * Closes client connections and removes clients when done.
         */
        for (int i = 0; i < client_count; ++i)
        {
            if (FD_ISSET(clientfd[i], &readfds))
            {
                int sNumber_client_tmp = data.sNumber;

                memset(&data, 0, sizeof(data)); 
                int valread = recv(clientfd[i], &data, sizeof(data), MSG_DONTWAIT);
                
                if (valread > 0)
                {
                    printf("s%d > %s\n", data.sNumber, data.text);

                    if (strcmp(data.text, "exit") == 0)
                    {
                        printf("[+] Closing connection for s%d\n", data.sNumber);
                        close(clientfd[i]);
                        clientfd[i] = 0;
                        client_count--;
                    }
                    else
                    {

                        printf("s%d > ", sNumber);
                        fgets(data.text, MAX_TEXT_LEN, stdin);
                        data.text[strcspn(data.text, "\n")] = '\0';
                        data.sNumber = sNumber;
                        send(clientfd[i], &data, sizeof(data), 0);
                    }
                }
                else if (valread == 0)
                {
                    if(sNumber_client_tmp == 0)
                         printf("Unknown cliented disconnected before sending a message.");
                    else
                        printf("[!] s%d disconnected unexpectedly.\n", sNumber_client_tmp);
                    close(clientfd[i]);
                    clientfd[i] = 0;
                    client_count--;
                }
            }
        }
           /*NON-BLOCKING STDIN Attempt
            if (FD_ISSET(STDIN_FILENO, &readfds))
            {
                char buf[1024] = {};
                buf[strcspn(buf, "\n")] = '\0';
                fgets(buf, 1023, stdin);
                printf("TEST: %s", buf);
            }
            */
       
    }

    /*
     * Closes any open client file descriptors by looping through
     * the clientfd array and calling close() on any descriptor > 0.
     */
    for (int i = 0; i < client_count; ++i)
    {
        if (clientfd[i] > 0)
        {
            close(clientfd[i]);
        }
    }

    /*
     * Closes the socket file descriptor. 
    */
    close(sockfd);
    return 0;
}