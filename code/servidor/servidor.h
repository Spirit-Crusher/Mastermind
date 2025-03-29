#include "../mastermind.h"
#include "core_gameplay.h"

/*############################### Protótipos ################################*/
void exit_handler();
void analise_move(game_t* game_pt);
void create_new_game(game_t* game, int socket, coms_t buffer_stream);
void stream_handler(int socket, game_t* game, coms_t buffer_stream);
void datagram_handler(int sd, struct sockaddr_un client_addr, socklen_t client_addrlen, coms_t buffer_dgram);
void save_game(rjg_t log) ;
void* thread_func_gameinstance(void* game_info);
void* thread_func_acceptgames();