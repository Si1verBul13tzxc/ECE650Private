#include "potato.h"
int main(int argc, char ** argv) {
  check_commands(argc, argv);
  const char * master_port = argv[1];
  int num_players = strtol(argv[2], NULL, 0);
  int num_hops = strtol(argv[3], NULL, 0);
  master_init_out(stdout, num_players, num_hops);
  int listener_fd = create_tcp_listener_fd(master_port);
  pollfd_t * pollfds = alloc_pollfds(listener_fd, num_players);
  player_info_t * players = alloc_player_infos(num_players);
  wait_palyer(pollfds, num_players + 1, players);
  init_players_listening_port(pollfds, num_players + 1, players);
  //test:
  /*for (int i = 0; i < num_players; i++) {
    int id = players[i].id;
    fprintf(stdout,
            "id: %d, hostname: %s, port: %s\n",
            id,
            players[i].hostname,
            players[i].listening_port);
            }*/
  //test code end

  send_neigher_info(pollfds, num_players + 1, players);
  free(players);
  receive_complete_message(pollfds, num_players + 1);
  play(pollfds, num_players + 1, num_hops);
  for (size_t i = 0; i < 4; i++) {
    close(pollfds[i].fd);
  }
  free(pollfds);
  return EXIT_SUCCESS;
}
