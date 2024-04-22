// Usage: ./client <IP_address> <port_number>

// The client attempts to connect to a server at the specified IP address and port number.
// The client should simultaneously do two things:
//     1. Try to read from the socket, and if anything appears, print it to the local standard output.
//     2. Try to read from standard input, and if anything appears, print it to the socket. 

// Remember that you can use a pthread to accomplish both of these things simultaneously.
