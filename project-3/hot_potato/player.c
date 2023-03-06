#include "potato.h"

int main(int argc, char ** argv) {
  if (argc != 3) {
    error_handle("Usage: ./player <hostname> <port_num>", 1, NULL, 0, NULL);
  }
  int ring_fd = connect_to_host(argv[1], argv[2]);
  if (ring_fd == -1) {
    return EXIT_FAILURE;
  }
  int listener = create_tcp_listener_fd("0");
  pollfd_t * pollfds = player_alloc_pollfds(listener, ring_fd);
  send_my_listening_port(pollfds);
  u_int32_t my_id = 0;
  u_int32_t num_players = 0;
  recv_neighbor_info_and_connect(pollfds, &my_id, &num_players);
  accept_right_neighbors(pollfds);
  //printf("done!\n");
  //char * message = "hi you left";
  //char message_buffer[15];
  //sprintf(message_buffer, "%d%s", my_id, message);
  //send(pollfds[3].fd, message_buffer, strlen(message_buffer), 0);
  //char buffer[15];
  //recv(pollfds[2].fd, buffer, 14, 0);
  //buffer[14] = 0;
  //printf("reveived: %s\n", buffer);
  notify_ringmaster_setupdone(pollfds[1].fd);
  player_play_potato(pollfds, my_id, num_players);
  for (size_t i = 0; i < 4; i++) {
    close(pollfds[i].fd);
  }
  free(pollfds);
  return EXIT_SUCCESS;
}
