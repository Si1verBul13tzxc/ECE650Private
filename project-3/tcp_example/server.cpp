#include <netdb.h>
#include <sys/socket.h>
#include <unistd.h>

#include <cstring>
#include <iostream>

using namespace std;

int main(int argc, char * argv[]) {
  int status;
  int socket_fd;
  struct addrinfo host_info;
  struct addrinfo * host_info_list;
  const char * hostname = NULL;
  const char * port = "4444";

  memset(&host_info, 0, sizeof(host_info));
  //initialize host_info to all 0.
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;  //for tcp
  host_info.ai_flags = AI_PASSIVE;      //for returned socket address suitable
  // for bind()ing that will accept() conncections.

  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  //If the AI_PASSIVE flag is specified in hints.ai_flags, and node is NULL, then the returned socket addresses will be suitable for bind(2)ing a socket that will accept(2) connections.
  if (status != 0) {
    cerr << "Error: cannot get address info for host" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  socket_fd = socket(host_info_list->ai_family,
                     host_info_list->ai_socktype,
                     host_info_list->ai_protocol);
  if (socket_fd == -1) {
    cerr << "Error: cannot create socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  int yes = 1;
  status = setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  // to reuse make the port avaialbe again, set reuse= yes! at socket level
  status = bind(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  //“assigning a name to a socket”
  if (status == -1) {
    cerr << "Error: cannot bind socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  status = listen(socket_fd, 100);
  //listen() marks the socket referred to by sockfd as a passive
  // socket, that is, as a socket that will be used to accept incoming
  // connection requests using accept(2).
  //100:maximum length to which the
  //queue of pending connections for sockfd may grow
  if (status == -1) {
    cerr << "Error: cannot listen on socket" << endl;
    cerr << "  (" << hostname << "," << port << ")" << endl;
    return -1;
  }  //if

  cout << "Waiting for connection on port " << port << endl;
  struct sockaddr_storage
      socket_addr;  // is a sockaddr, It can be used to embed sufficient storage
  //for a sockaddr of any type within a larger structure.
  socklen_t socket_addr_len = sizeof(socket_addr);
  int client_connection_fd;
  client_connection_fd =
      accept(socket_fd, (struct sockaddr *)&socket_addr, &socket_addr_len);
  //It extracts the first
  //connection request on the queue of pending connections for the
  //listening socket, sockfd, creates a new connected socket, and
  //returns a new file descriptor referring to that socket.  The
  //newly created socket is not in the listening state.  The original
  //socket sockfd is unaffected by this call.
  if (client_connection_fd == -1) {
    cerr << "Error: cannot accept connection on socket" << endl;
    return -1;
  }  //if
  char buffer[512];
  recv(client_connection_fd, buffer, 9, 0);
  buffer[9] = 0;

  cout << "Server received: " << buffer << endl;

  freeaddrinfo(host_info_list);
  close(socket_fd);

  return 0;
}
