// Usage: ./server <port_number>

// This server should listen on the specified port, waiting for incoming connections.
// After a client connects, add it to a list of currently connected clients.
// If any message comes in from *any* connected client, then it is repeated to *all*
//    other connected clients.
// If reading or writing to a client's socket fails, then that client should be removed
      from the linked list. 

// Remember that blocking read calls will cause your server to stall. Instead, set your
// your sockets to be non-blocking. Then, your reads will never block, but instead return
// an error code indicating there was nothing to read- this error code can be either
// EAGAIN or EWOULDBLOCK, so make sure to check for both. If your read call fails
// with that error, then ignore it. If it fails with any other error, then treat that
// client as though they have disconnected.

// You can create non-blocking sockets by passing the SOCK_NONBLOCK argument to both
// the socket() function, as well as the accept4() function.
