#include "potato.h"

void error_handle(const char * message,
                  int exit_,
                  pollfd_t * pollfds,
                  size_t poll_size,
                  player_info_t * players) {
  if (message != NULL) {
    fprintf(stderr, "%s", message);
  }
  if (exit_) {
    if (pollfds != NULL) {
      for (size_t i = 0; i < poll_size; i++) {
        close(pollfds[i].fd);
      }
    }
    free(pollfds);
    free(players);
    exit(EXIT_FAILURE);
  }
}

int create_tcp_listener_fd(const char * port) {  //reference from Beej's Guide
  int listener_fd = -1;
  int status = -1;
  addrinfo_t host_info_hints;
  addrinfo_t * host_info_list;
  addrinfo_t * host_ptr;

  memset(&host_info_hints, 0, sizeof(host_info_hints));
  host_info_hints.ai_family = AF_UNSPEC;
  host_info_hints.ai_socktype = SOCK_STREAM;  //TCP
  host_info_hints.ai_flags = AI_PASSIVE;

  status = getaddrinfo(NULL, port, &host_info_hints, &host_info_list);
  if (status != 0) {
    error_handle("cannot getaddrinfo", 1, NULL, 0, NULL);
  }

  for (host_ptr = host_info_list; host_ptr != NULL; host_ptr = host_ptr->ai_next) {
    listener_fd =
        socket(host_ptr->ai_family, host_ptr->ai_socktype, host_ptr->ai_protocol);
    if (listener_fd == -1) {
      continue;
    }
    int yes = 1;
    setsockopt(listener_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
    status = bind(listener_fd, host_ptr->ai_addr, host_ptr->ai_addrlen);
    if (status == -1) {
      continue;
    }
    break;  //successful create socket and bind
  }

  freeaddrinfo(host_info_list);
  if (host_ptr == NULL) {
    error_handle("failure to create or bind socket", 1, NULL, 0, NULL);
  }

  status = listen(listener_fd, 20);
  if (status == -1) {
    error_handle("cannot listen on socket", 1, NULL, 0, NULL);
  }

  return listener_fd;
}

int connect_to_host(const char * theHostname,
                    const char * thePort) {  //reference from Beej's Guide
  int status;
  int socket_fd;
  addrinfo_t host_info;
  addrinfo_t * host_info_list;
  addrinfo_t * host_ptr;
  const char * hostname = theHostname;
  const char * port = thePort;
  memset(&host_info, 0, sizeof(host_info));
  host_info.ai_family = AF_UNSPEC;
  host_info.ai_socktype = SOCK_STREAM;
  status = getaddrinfo(hostname, port, &host_info, &host_info_list);
  if (status != 0) {
    return -1;
  }

  for (host_ptr = host_info_list; host_ptr != NULL; host_ptr = host_ptr->ai_next) {
    socket_fd = socket(host_ptr->ai_family, host_ptr->ai_socktype, host_ptr->ai_protocol);
    if (socket_fd == -1) {
      continue;
    }
    break;  //successful create socket and bind
  }
  freeaddrinfo(host_info_list);
  if (host_ptr == NULL) {
    return -1;
  }

  status = connect(socket_fd, host_info_list->ai_addr, host_info_list->ai_addrlen);
  if (status == -1) {
    return -1;
  }
  return socket_fd;
}

pollfd_t * alloc_pollfds(int listener, int ring_fd) {
  pollfd_t * pollfds = malloc(sizeof(*pollfds) *
                              (4));  // one for ring_fd, one for listener, two for players
  pollfds->fd = listener;
  pollfds->events = POLLIN;
  pollfds[1].fd = ring_fd;
  pollfds[1].events = POLLIN;
  return pollfds;
}

u_int16_t * get_sockaddr(struct sockaddr * saddr) {
  // reference: Beej's Guide to Network Programming
  if (saddr->sa_family == AF_INET) {
    return &(((struct sockaddr_in *)saddr)->sin_port);
  }
  else {
    return &(((struct sockaddr_in6 *)saddr)->sin6_port);
  }
}

void send_port_num(int ring_fd, const char * port) {
  int len = strlen(port);
  char buffer[10];
  sprintf(buffer, "%d%s", len, port);
  len = strlen(buffer);
  sendall(ring_fd, buffer, &len);
}

void send_my_listening_port(pollfd_t * pollfds) {
  struct sockaddr_storage my_socket_addr;
  socklen_t len = sizeof(my_socket_addr);
  int status = getsockname(pollfds[0].fd, (struct sockaddr *)&my_socket_addr, &len);
  if (status != 0) {
    error_handle("fail get listening port", 1, pollfds, 4, NULL);
  }
  u_int16_t port = ntohs(*get_sockaddr((struct sockaddr *)&my_socket_addr));
  char buffer[10] = {0};
  sprintf(buffer, "%hu", port);
  //  fprintf(stdout, "my listening port is %s\n", buffer);
  send_port_num(pollfds[1].fd, buffer);
}

void accept_one_neighbor(pollfd_t * pollfds) {
  struct sockaddr_storage client_socket_addr;
  socklen_t len = sizeof(client_socket_addr);
  int new_fd = accept(pollfds[0].fd, (struct sockaddr *)&client_socket_addr, &len);
  if (new_fd == -1) {
    error_handle("cannot accept connection on socket", 1, pollfds, 4, NULL);
  }
  pollfds[2].fd = new_fd;  //2 is right neighbor
  pollfds[2].events = POLLIN;
}

void * accept_right_neighbors(void * fds) {
  pollfd_t * pollfds = (pollfd_t *)fds;
  while (1) {
    if (poll(pollfds, 4, -1) == -1) {
      error_handle("poll fail", 1, pollfds, 4, NULL);
    }
    if (pollfds[0].revents & POLLIN) {
      accept_one_neighbor(pollfds);
      break;
    }
  }
  return NULL;
}

void recv_neighbor_info_and_connect(void * fds,
                                    u_int32_t * my_id,
                                    u_int32_t * num_players) {
  pollfd_t * pollfds = (pollfd_t *)fds;
  neighbors_info_t neighbors;
  while (1) {
    int poll_count = poll(pollfds, 4, -1);
    if (poll_count <= 0) {
      error_handle("poll fail", 1, pollfds, 4, NULL);
    }
    if (pollfds[1].revents & POLLIN) {  //listen from ring master
      recv(pollfds[1].fd, &neighbors, sizeof(neighbors_info_t), MSG_WAITALL);
      /*printf("my id is %d, there is totally %d players in this game,left neighbor host "
             "is : %s, port: %s\n",
             ntohl(neighbors.id),
             ntohl(neighbors.num_players),
             neighbors.left_hostname,
             neighbors.left_port);
      */
      break;
    }
  }
  int left_sock = connect_to_host(neighbors.left_hostname, neighbors.left_port);
  if (left_sock == -1) {
    error_handle("connect to left neighbor fail/n", 1, pollfds, 4, NULL);
  }
  pollfds[3].fd = left_sock;
  pollfds[3].events = POLLIN;
  *my_id = ntohl(neighbors.id);
  *num_players = ntohl(neighbors.num_players);
  fprintf(
      stdout, "Connected as player %d out of %d total players\n", *my_id, *num_players);
}

void notify_ringmaster_setupdone(int ring_fd) {
  const char * buffer = "D";
  send(ring_fd, buffer, strlen(buffer), 0);
}

int check_potato(potato_t * p, char my_id) {
  u_int32_t num_hops = ntohl(p->num_hops);
  num_hops--;
  p->num_hops = htonl(num_hops);
  u_int32_t cur_hops = ntohl(p->cur_hops);
  p->trace[cur_hops++] = my_id;
  p->cur_hops = htonl(cur_hops);
  if (num_hops == 0) {
    fprintf(stdout, "%s\n", "I'm it");
    return 1;
  }
  else {
    return 0;
  }
}

int sendall(int sock_fd, char * buf, int * len) {
  //reference from Beej's Guide
  int total = 0;         // how many bytes we've sent
  int bytesleft = *len;  // how many we have left to send
  int n;
  while (total < *len) {
    n = send(sock_fd, buf + total, bytesleft, 0);
    if (n == -1) {
      break;
    }
    total += n;
    bytesleft -= n;
  }
  *len = total;             // return number actually sent here
  return n == -1 ? -1 : 0;  // return -1 on failure, 0 on success
}

void player_play_potato(pollfd_t * pollfds, char my_id, int num_players) {
  int finish = 0;
  srand((unsigned int)time(NULL) + my_id - '0');
  while (!finish) {
    if (poll(pollfds, 4, -1) == -1) {
      error_handle("poll fail", 1, pollfds, 4, NULL);
    }
    for (int i = 1; i < 4; i++) {
      if (pollfds[i].revents & POLLIN) {
        potato_t p;
        int bytes = recv(pollfds[i].fd, &p, sizeof(potato_t), MSG_WAITALL);
        if (bytes == 0) {
          return;
        }
        int status = check_potato(&p, my_id);
        if (status == 1) {
          int len = sizeof(p);
          sendall(pollfds[1].fd, (char *)&p, &len);
          finish = 1;
        }
        else {
          int random = rand() % 2;
          int pollfds_index = random + 2;
          int neighbor_id;
          if (pollfds_index == 2) {  //right neighor
            neighbor_id = (my_id - '0' + 1 + num_players) % num_players;
          }
          else {  //==3, left neighbor
            neighbor_id = (my_id - '0' - 1 + num_players) % num_players;
          }
          int len = sizeof(p);
          sendall(pollfds[2 + random].fd, (char *)&p, &len);
          printf("Sending potato to %d\n", neighbor_id);
        }
      }
    }
  }
}

void init_players_listening_port(pollfd_t * pollfds,
                                 size_t poll_size,
                                 player_info_t * players) {
  size_t setup_player = 0;
  while (1) {
    int poll_count = poll(pollfds, poll_size, -1);
    if (poll_count < 0) {  //-1 timeout=forever
      error_handle("poll fail", 1, pollfds, poll_size, players);
    }
    for (size_t i = 1; i < poll_size; i++) {
      if (pollfds[i].revents & POLLIN) {  //client send!
        //printf("set up player %zu\n", i);
        recv_listening_port(pollfds[i].fd, players + (i - 1));
        setup_player++;
      }
    }
    //printf("poll size is %zu\n", poll_size);
    //printf("player num is %zu\n", setup_player);
    if (setup_player >= poll_size - 1) {
      break;
    }
  }
}

void recv_listening_port(int fd, player_info_t * player) {
  char size_buf[2];
  recv(fd, size_buf, 1, 0);
  size_buf[1] = 0;
  size_t size = strtol(size_buf, NULL, 0);
  //printf("size is :%zu\n", size);
  char content_buf[size + 1];
  recv(fd, content_buf, size, 0);
  content_buf[size] = 0;
  //printf("content is:%s\n", content_buf);
  strcpy(player->listening_port, content_buf);
}

void check_commands(int argc, char ** argv) {
  if (argc != 4) {
    fprintf(stderr, "%s", "Usage: ./ringmaster <port_num> <num_players> <num_hops>\n");
    exit(EXIT_FAILURE);
  }
  int num_players = 0;
  if ((num_players = strtol(argv[2], NULL, 0)) <= 1) {
    fprintf(stderr, "%s", "num_palyers must be greater than 1\n");
    exit(EXIT_FAILURE);
  }
  int num_hops = strtol(argv[3], NULL, 0);
  if (num_hops < 0 || num_hops > 512) {
    fprintf(stderr, "%s", "num_hops must between 0 to 512 (inclusively)\n");
    exit(EXIT_FAILURE);
  }
}

void master_init_out(FILE * f, int num_players, int num_hops) {
  fprintf(f, "%s", "Potato Ringmaster\n");
  fprintf(f, "Players = %d\n", num_players);
  fprintf(f, "Hops = %d\n", num_hops);
}

player_info_t * alloc_player_infos(int num_players) {
  return malloc(sizeof(player_info_t) * num_players);
}

void wait_palyer(pollfd_t * pollfds, size_t poll_size, player_info_t * players) {
  int setup_ready = 0;
  size_t poll_item_count = 1;  //has only listener now
  while (!setup_ready) {
    if (poll(pollfds, poll_item_count, -1) == -1) {  //-1 timeout=forever
      error_handle("poll fail", 1, pollfds, poll_size, players);
    }
    if (pollfds[0].revents & POLLIN) {  //listener ready to read
      //printf("%s", "one player connected in!\n");
      accept_new_fd(pollfds, &poll_item_count, players);
    }
    if (poll_item_count == poll_size) {  // all players connected
      setup_ready = 1;
    }
  }
}

void accept_new_fd(pollfd_t * pollfds,
                   size_t * poll_item_count,
                   player_info_t * players) {
  struct sockaddr_storage client_socket_addr;
  socklen_t len = sizeof(client_socket_addr);
  int new_fd = accept(pollfds[0].fd, (struct sockaddr *)&client_socket_addr, &len);
  if (new_fd == -1) {
    error_handle(
        "cannot accept connection on socket", 1, pollfds, *poll_item_count, players);
  }
  pollfds[*poll_item_count].fd = new_fd;
  pollfds[*poll_item_count].events = POLLIN;
  //set up player info
  players[*poll_item_count - 1].id = *poll_item_count - 1;
  char * hostname = players[*poll_item_count - 1].hostname;
  getnameinfo((struct sockaddr *)&client_socket_addr, len, hostname, 128, NULL, 0, 0);
  (*poll_item_count)++;
}

void send_neigher_info(pollfd_t * pollfds, size_t poll_size, player_info_t * players) {
  size_t num_players = poll_size - 1;
  for (size_t i = 1; i < poll_size; i++) {
    int left_id = (i - 1 + num_players - 1) % num_players;
    neighbors_info_t neighbors;
    neighbors.id = htonl(players[i - 1].id);
    neighbors.num_players = htonl(poll_size - 1);
    strcpy(neighbors.left_hostname, players[left_id].hostname);
    strcpy(neighbors.left_port, players[left_id].listening_port);
    int len = sizeof(neighbors);
    sendall(pollfds[i].fd, (char *)&neighbors, &len);
  }
}

void receive_complete_message(pollfd_t * pollfds, size_t poll_size) {
  size_t ready_to_player_num = 0;
  while (ready_to_player_num != poll_size - 1) {
    if (poll(pollfds, poll_size, -1) == -1) {  //-1 timeout=forever
      error_handle("poll fail", 1, pollfds, poll_size, NULL);
    }
    for (size_t i = 0; i < poll_size; i++) {
      if (i != 0 && pollfds[i].revents & POLLIN) {  //ready to read
        char buffer[2];
        recv(pollfds[i].fd, buffer, sizeof(buffer), 0);
        //fprintf(stdout, "%s\n", buffer);
        printf("Player %zu is ready to play\n", i - 1);
        ready_to_player_num++;
      }
    }
  }
}

void play(pollfd_t * pollfds, size_t poll_size, int num_hops) {
  if (num_hops == 0) {
    return;
  }
  potato_t p;
  memset(&p, 0, sizeof(potato_t));
  p.num_hops = htonl(num_hops);
  p.cur_hops = htonl(0);
  //send to randomly a choose player
  srand((unsigned int)time(NULL) + poll_size - 1);
  int random = rand() % (poll_size - 1);
  int len = sizeof(potato_t);
  //printf("size of potato is %d\n", len);
  fprintf(stdout, "Ready to start the game, sending potato to player %d\n", random);
  sendall(pollfds[1 + random].fd, (char *)&p, &len);
  //printf("ringmaster send to %d, sent is %d\n", random + 1, len);
  int finish = 0;
  while (!finish) {
    if (poll(pollfds, poll_size, -1) == -1) {
      error_handle("poll fail", 1, pollfds, poll_size, NULL);
    }
    for (size_t i = 0; i < poll_size; i++) {
      if (pollfds[i].revents & POLLIN) {
        potato_t received_potato;
        recv(pollfds[i].fd, &received_potato, sizeof(potato_t), MSG_WAITALL);
        print_potato(&received_potato);
        finish = 1;
        break;
      }
    }
  }
}

void print_potato(potato_t * p) {
  char * trace = p->trace;
  char my_trace[2048] = {0};
  size_t trace_cur = 0;
  size_t my_trace_cur = 0;
  while (trace[trace_cur] != '\0') {
    my_trace[my_trace_cur] = trace[trace_cur];
    my_trace[my_trace_cur + 1] = ',';
    my_trace_cur += 2;
    trace_cur += 1;
  }
  my_trace[my_trace_cur - 1] = '\0';
  fprintf(stdout, "Trace of potato:\n");
  fprintf(stdout, "%s\n", my_trace);
}
