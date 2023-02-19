#include <arpa/inet.h>
#include <netdb.h>
#include <poll.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

typedef struct addrinfo addrinfo_t;
typedef struct pollfd pollfd_t;
struct player_info {
  int id;
  char hostname[128];
  char listening_port[10];
};
typedef struct player_info player_info_t;
struct neighbors_info {
  u_int32_t id;
  u_int32_t num_players;
  char left_hostname[128];
  char left_port[10];
};
typedef struct neighbors_info neighbors_info_t;

struct potato_tag {
  u_int32_t num_hops;
  u_int32_t cur_hops;
  char trace[513];  //512 traces and \0
};

typedef struct potato_tag potato_t;

void check_commands(int argc, char ** argv);
void master_init_out(FILE * f, int num_players, int num_hops);
void error_handle(const char * message,
                  int exit_,
                  pollfd_t * pollfds,
                  size_t poll_size,
                  player_info_t * players);
int create_tcp_listener_fd(const char * port);
pollfd_t * alloc_pollfds(int listener_fd, int num_players);
void wait_palyer(pollfd_t * pollfds, size_t poll_size, player_info_t * players);
void accept_new_fd(pollfd_t * pollfds, size_t * poll_item_count, player_info_t * players);
player_info_t * alloc_player_infos(int num_players);
u_int16_t * get_sockaddr(struct sockaddr * saddr);
void init_players_listening_port(pollfd_t * pollfds,
                                 size_t poll_size,
                                 player_info_t * players);
void recv_listening_port(int fd, player_info_t * player);
void send_neigher_info(pollfd_t * pollfds, size_t poll_size, player_info_t * players);
void receive_complete_message(pollfd_t * pollfds, size_t poll_size);
void play(pollfd_t * pollfds, size_t poll_size, int num_hops);
int sendall(int sock_fd, char * buf, int * len);
void print_potato(potato_t * p);
int connect_to_host(const char * theHostname, const char * thePort);
void send_my_listening_port(pollfd_t * pollfds);
void recv_neighbor_info_and_connect(void * fds,
                                    u_int32_t * my_id,
                                    u_int32_t * num_players);
void * accept_right_neighbors(void * fds);
void notify_ringmaster_setupdone(int ring_fd);
void player_play_potato(pollfd_t * pollfds, char my_id, int num_players);
