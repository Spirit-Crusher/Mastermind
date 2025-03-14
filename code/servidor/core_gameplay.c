#include "core_gameplay.h"

//esta será a thread que espera novos jogos (tenho de criar comunição com clientes) COLOCAR NO SERVIDOR.C
#define SERVERNAME "/tmp/GAME_SERVER"
#define MSG "SERVER IS ALIVE AND WELL"

void wait_for_games()
{
    int sd; //socket descriptor do servidor
    int s; //client specific socket descriptor
    int sd, new_sock, max_sd, activity, i;
    struct sockaddr_un server_addr;
    socklen_t server_addrlen;
    struct sockaddr_un client_addr;
    socklen_t client_addrlen; //não será igual ao server_addrlen?
    char buffer[100];
    fd_set rfds; //set dos descriptors das sockets
    int client_sockets_fd[NJMAX] = {0}; //array com socket descriptors de todos os clientes

    //criação da socket do servidor
    if((sd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ) 
    {
        perror("Erro a criar socket"); 
        exit(-1);
    }

    //inicialização da estrutura para associar (bind) server à socket
    server_addr.sun_family = AF_UNIX;
    memset(server_addr.sun_path, 0, sizeof(server_addr.sun_path)); //tenho de inicializar desta maneira? Não posso só fazer strcpy diretamente?
    strcpy(server_addr.sun_path, SERVERNAME);   //definir path para a socket (vai ficar no /tmp)
    server_addrlen = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);

    unlink(SERVERNAME); //remover ficheiro socket caso este já tenha sido criado previamente

    //ligar server à socket (bind)
    if(bind(sd, (struct sockaddr *) &server_addr, server_addrlen) < 0 ) 
    {
        perror("Erro no bind"); 
        exit(-1);
    }

    //socket vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if(listen(sd, NJMAX) < 0 ) 
    {
        perror("Erro no listen"); 
        exit(-1);
    }

    printf("À espera de receber conexões dos clientes...\n");

    //receber datagramas
    while(true)
    {
        FD_ZERO(&rfds); //limpar set
        FD_SET(sd, &rfds); //adicionar socket que está listening ao set (para select conseguir ver se temos novos clientes)
        max_sd = sd; //temos de guardar esta informação (estava na man page, não sei qual é o propósito)

        //adicionar sockets dos clientes ao set
        for(i = 0; i < NJMAX; ++i) 
        {
            if(client_sockets_fd[i] > 0) //adicionar ao set se file descriptor existe (>0)
            {
                FD_SET(client_sockets_fd[i], &rfds);
                if(client_sockets_fd[i] > max_sd) max_sd = client_sockets_fd[i]; //atualizar qual é o sd mais recente (valor maior)
            }
        }

        //esperar por atividade de qualquer um dos clientes (bloqueia)
        if((activity = select(max_sd + 1, &rfds, NULL, NULL, NULL)) < 0 && errno != EINTR)
        {
            perror("Erro no select");
        }

        //há atividade, vamos aceitar conexão
        if(FD_ISSET(sd, &rfds)) 
        {
            client_addrlen = sizeof(client_addr);
            if((new_sock = accept(sd, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) 
            {
                perror("Erro no accept");
                exit(-1);
            }

            printf("Novo cliente conectado\n");

            // Adicionar o novo cliente ao array de sockets
            for(i = 0; i < NJMAX; ++i) 
            {
                if(client_sockets_fd[i] == 0) //procurar "espaço vazio" no array
                {
                    client_sockets_fd[i] = new_sock;
                    printf("Socket guardada no indice: %d\n", i);
                    break; //se já encontrei não vale a pena continuar a iterar
                }
            }
        }

        //ler datagramas dos clientes
        for(i = 0; i < NJMAX; ++i)
        {
            s = client_sockets_fd[i]; //atualizar o cliente que estamos a ler

            //ler mensagem do cliente
            if(read(s, buffer, sizeof(buffer)) <= 0) 
            {
                perror("Cliente desconectou-se");
                close(s);
                client_sockets_fd[i] = 0;
            }
            else 
            {
                printf("SERVER: Recebi: %s\n", buffer); //mostrar o que foi recebido
                
                //enviar algo para o cliente
                if(write(s, MSG, strlen(MSG)+1) < 0)    
                {
                    perror("Erro no write");
                }
            }
        }
    }
    
    //NAO ESQUECER DE FECHAR SOCKET E DAR UNLINK
    close(sd);
    /*close(s);*/ //aqui já não é ncessário fazer porque fizemos no loop sempre que cliente se disconecta
    unlink(SERVERNAME);
}

void run_the_game()
{

}

//inicialização do jogo
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

    return game;
}

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};

    // incementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (int i = 0; i < game_pt->n_char; ++i)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            (np)++;
            used_secret[i] = true;
            used_guess[i] = true;
        }
    }

    // procurar por letras certas no sítio errado
    for (int i = 0; i < game_pt->n_char; ++i)
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
