#include "core_gameplay.h"

//esta será a thread que espera novos jogos (tenho de criar comunição com clientes) COLOCAR NO SERVIDOR.C
#define SERVERNAME "/tmp/GAME_SERVER"
void wait_for_games()
{
    int sd; //socket descriptor
    struct sockaddr_un server_addr;
    socklen_t server_addrlen;
    struct sockaddr_un client_addr;
    socklen_t client_addrlen;
    char buffer[100];

    //criação da socket
    if ((sd = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("Erro a criar socket"); 
        exit(-1);
    }

    //inicialização da estrutura para associar (bind) server à socket
    server_addr.sun_family = AF_UNIX;
    memset(server_addr.sun_path, 0, sizeof(server_addr.sun_path)); //tenho de inicializar desta maneira? Não posso só fazer strcpy diretamente?
    strcpy(server_addr.sun_path, SERVERNAME);   //definir path para a socket (vai ficar no /tmp)
    server_addrlen = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);

    //ligar server à socket (bind)
    if (bind(sd, (struct sockaddr *) &server_addr, server_addrlen) < 0 ) 
    {
        perror("Erro no bind"); 
        exit(-1);
    }

    //receber datagramas
    while (true)
    {
        client_addrlen = sizeof(client_addr);
        if (recvfrom(sd, buffer, sizeof(buffer), 0, (struct sockaddr *) & client_addr, &client_addrlen) < 0) 
        {
            perror("Erro no recvfrom");
        }
        else 
        {
            printf("SERV: Recebi: %s\n", buffer); //mostrar o que foi recebido
            //enviar algo para o cliente
            //if (sendto(sd, MSG, strlen(MSG)+1, 0, (struct sockaddr *)&client_addr, client_addrlen) < 0) perror("Erro no sendto");
        }
    }
    
    //NAO ESQUECER DE FECHAR SOCKET E DAR UNBIND
}

//esta é chamada depois da thread aceitar um novo jogo
game_t create_new_game(char* player_name, int dificulty)
{
    game_t game = { .correct_sequence = "AAAA", 
                    .log.nd = dificulty, 
                    /*.log.nj = , para isto faço strcpy*/ 
                    .log.nt = 0, 
                    .log.ti = time(NULL), 
                    .log.tf = NO_TIME_REGISTERED, //inicializei com alguma coisa para conseguir perceber se der problemas
                    .n_char = 4, 
                    .nt_max = 3, 
                    .player_move = "BCAA", 
                    .np = 0, 
                    .nb = 0, 
                    .game_state = ONGOING,
                };
    
    strcpy(&game.log.nj, player_name); //inicializa o nome do jogador


}

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};

    // incementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (int i = 0; i < game_pt->n_char; i++)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            (np)++;
            used_secret[i] = true;
            used_guess[i] = true;
        }
    }

    // procurar por letras certas no sítio errado
    for (int i = 0; i < game_pt->n_char; i++)
    {
        if (!used_guess[i])
        { // se esta posição não teve uma ligação direta
            for (int j = 0; j < game_pt->n_char; j++)
            {
                if (!used_secret[j] && game_pt->correct_sequence[i] == game_pt->player_move[j])
                {
                    nb++;
                    used_secret[j] = true;
                    break;
                }
            }
        }
    }

    // guardar nº de respostas certas no sítio certo/errado
    game_pt->nb = nb;
    game_pt->np = np;

    // decrementar o nº de jogadas que faltam

    // verificar se o jogo acabou
    if (np == game_pt->n_char)
    {
        // jogador ganhou
        return game_pt->game_state = PLAYER_WIN;
    }
    else if (game_pt->nt_max == ++(game_pt->log.nt))
    {
        // jogador sem mais tentativas, perdeu
        return game_pt->game_state = PLAYER_LOST;
    }

    // jogo continua
    return ONGOING;
}
