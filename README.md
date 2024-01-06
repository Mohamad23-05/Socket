Task for groups of 2:
Create a Sender (Client) and a Receiver (Server) program in C (console applications) to transmit text messages from the Sender to the Receiver via TCPv6/IPv6. 
Both the Sender and the Receiver should be capable of sending and receiving data. The Sender will receive the IPv6 address and the port of the Receiver, as well as your sNumber for identification, as program arguments. 
Similarly, the Receiver should be configurable via program arguments, receiving the sNumber of your communication partner and the port to listen on.

Task Breakdown:
Sender:
- Initialize the Sender by creating a TCP socket.
- Utilize the `select()` function. Depending on the return, proceed to the next step or stop.
- Read the text provided via the console.
- Build the packet to be sent (use a data structure like `struct packet { char text[...]; ... sNumber;}`).
- Define this structure in a separate header file. Assume a text with a maximum of 1024 characters.
- Note: You can directly pass this structure to the TCP socket function `send()` without serialization.
- Possible reception (`recv()`) of a message from the communication partner: Output the identification (sNumber) and the received message on the console, for instance, in the format `s12345 > Hello ...`.

Receiver:
- Initialize the TCP socket (don't forget `bind()`).
- Otherwise, the process will be similar to the Sender's, especially regarding receiving messages.

Note: Both the Client and Server have similar functionality. The main difference lies in the invocation of `bind()` and `listen()`, hence the execution order of the two application instances. 
Start the Server first to listen for incoming connections from the Client.

Hint: Work without additional threads! Achieve simultaneous sending and receiving using the `select()` function, allowing for asynchronous sockets. 
This function helps distinguish between data arriving via the keyboard (`stdin`) or the network (socket file descriptor). You can then either send the data to the Receiver or read the data from the socket.
