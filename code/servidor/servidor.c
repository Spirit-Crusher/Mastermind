#pragma pack(1)

#include "mastermind.h"

coms_t buffer_dgram; //posso usar só 1 buffer com mutex I think mas acho que ia tornar tudo mais lento sem necessidade e o prof não especifica
pthread_t thread_acceptgames;
pthread_t thread_gameinstance[NJMAX]; //acho que isto pode ser local ao acceptgames
game_t* game_instances[NJMAX] = {0};
struct sockaddr_un client_addr;
socklen_t client_addrlen; //não será igual ao server_addrlen? (acho que posso eliminar esta variável e passar o server_addrlen para addrlen)
rules_t game_rules = {.maxj = MAXNJ, .maxt = MAXT*60};


void exit_handler()
{
    exit(0);
}

/*####################################################### analise_move ######################################################*/

void analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};
    int i;

    // incrementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (i = 0; i < game_pt->n_char; ++i)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            np++;
            used_secret[i] = true;
            used_guess[i] = true;
        }
    }

    // procurar por letras certas no sítio errado
    for (i = 0; i < game_pt->n_char; ++i)
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
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_WIN;
    }
    else if (game_rules.maxj <= game_pt->log.nt)
    {
        // jogador sem mais tentativas, perdeu
        game_pt->log.tf = time(NULL);
        game_pt->game_state = PLAYER_LOST;
    }
    else
    {   // jogo continua
        game_pt->game_state = ONGOING;
    }

    return;
}


/*####################################################### create_new_game ######################################################*/

void create_new_game(game_t* game, int game_number, int socket, coms_t buffer_stream)
{
    game->log.nd = buffer_stream.arg2.n; //dificuldade
    strcpy(game->log.nj, buffer_stream.arg1.Name); //nome do jogador
    game->log.nt = 0;
    game->log.ti = time(NULL);
    game->log.tf = NO_TIME_REGISTERED; //inicializei com alguma coisa para conseguir perceber se der problemas

    printf("\ndif: %d\n\n", buffer_stream.arg2.n);
    printf("\nname: %s\n\n", buffer_stream.arg1.Name);

    if (buffer_stream.arg2.n == DIFF_1)
    {
        game->n_char = 3;
        char key[game->n_char+1];
        generate_key(key, DIFF_1);
        printf("[INFO] Key:%s\n", key);
        if(strcpy(game->correct_sequence, key) != NULL) printf("[INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else if (buffer_stream.arg2.n == DIFF_2)
    {
        game->n_char = 5;
        char key[game->n_char+1];
        generate_key(key, DIFF_2);
        printf("[INFO] Key: %s\n", key);
        if(strcpy(game->correct_sequence, key) != NULL) printf("[INFO] Key %s guardada com sucesso\n", game->correct_sequence); //inicializa o código secreto
    }
    else
    {
        printf("[ERRO] Problemas a gerar a key\n"); 
        exit(-1);
    }

    //game->player_move = "0000"; desnecessário (provavelmente), also tem de ser com strcpy

    game->np = 0;
    game->nb = 0;
    game->game_state = ONGOING;

    //inicializar novas vars que criei
    game->sd = socket;

    printf("[INFO] Init do jogo funcionou\n");

    return;
}


/*####################################################### stream_handler ######################################################*/

void stream_handler(int socket, int game_number, game_t* game, coms_t buffer_stream)
{
    if (buffer_stream.command == JG)
    {
        //enviar jogada associada ao socket de onde foi recebida stream para a thread de jogo adequada
        printf("[INFO] SERVER_STREAM: O jogador com descriptor %d deseja fazer a seguinte jogada: %s\n", socket, buffer_stream.arg1.move);

        strcpy(game->player_move, buffer_stream.arg1.move);
        printf("Move: %s    Correct move: %s\n", game->player_move, game->correct_sequence);
        analise_move(game);
        printf("[INFO] Jogada analisada com sucesso\n");
        printf("Gamestate = %d\n", game->game_state);
    }
    else
    {
        printf("[ERRO] Ops... Isto não era suposto acontecer.\n");
        exit(-1);
    }
    
    return;
}


/*####################################################### datagram_handler ######################################################*/

void datagram_handler(int sd, struct sockaddr_un client_addr, socklen_t client_addrlen)
{
    char buffer_send[50];

    switch (buffer_dgram.command)
    {
        case CLM:
            sprintf(buffer_send, "%d:%d", game_rules.maxj, game_rules.maxt);
            if(sendto(sd, buffer_send, strlen(buffer_send), 0, (struct sockaddr*) &client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }
            break;

        case MLM:
            game_rules.maxj = buffer_dgram.arg1.j;
            game_rules.maxt = buffer_dgram.arg2.t;
            
            sprintf(buffer_send, "Game rules changed.\n");
            if(sendto(sd, buffer_send, strlen(buffer_send), 0, (struct sockaddr*) &client_addr, client_addrlen) < 0)
            {
                perror("[ERRO] Erro no envio de datagrama");
            }
            break;

        case CER:
            //code
            break;

        case AER:
            //code
            break;

        case DER:
            //code
            break;

        case TMM:
            exit_handler();
            break;

        default:
            break;
    }
}


/*####################################################### thread_func_gameinstance ######################################################*/

void* thread_func_gameinstance(void* game_info)
{   
    new_game_info info = (*(new_game_info*) game_info);
    coms_t buffer_stream = info.buffer_s;
    int game_number = info.game_number;
    int socket = info.sd;
    int stream = info.sock_stream;
    char result[strlen(MOVE_REGISTERED)+8+1]; //!melhorar isto

    printf("[INFO] Esta é a socket e o game number do novo jogador: %d:%d\n", socket, game_number);

    game_t* current_game = NULL;

    current_game = malloc(sizeof(game_t));
    if (current_game == NULL) 
    {
        perror("[ERRO] Falha a alocar estrutura do jogo");
        exit(-1);
    }

    game_instances[game_number] = current_game;

    create_new_game(current_game, game_number, socket, buffer_stream);
    printf("[INFO] Jogo nº%d foi iniciado\n", game_number);

    do
    {
        if(read(socket, &buffer_stream, sizeof(buffer_stream)) <= 0) 
        {
            perror("[AVISO] Cliente desconectou-se");
            close(socket);
            free(current_game);
            game_instances[game_number] = NULL;
            return NULL;
        }

        
        printf("STATE1 = %d\n", current_game->game_state);
        stream_handler(socket, game_number, current_game, buffer_stream); //processar request

        printf("STATE2 = %d\n", current_game->game_state);

        switch (current_game->game_state)
        {
            case ONGOING:
                //mostrar resultado da jogada
                sprintf(result, "nb:%d np:%d", current_game->nb, current_game->np);
                printf("Resultado da jogada: %s\n", result);
                if(write(socket, result, strlen(result)+1) < 0)    
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;

            case PLAYER_LOST:
                printf("[INFO] O jogador perdeu\n");
                if(write(socket, GAME_LOST, strlen(GAME_LOST)+1) < 0)    
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;
            
            case PLAYER_WIN:
                printf("[INFO] O jogador ganhou\n");
                if(write(socket, GAME_WON, strlen(GAME_WON)+1) < 0)    
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;

            default:
                if(write(socket, GAME_CRASHED, strlen(GAME_CRASHED)+1) < 0)    
                {
                    perror("[ERRO] Erro no envio de stream");
                }
                break;
        }
        
    } while(current_game->game_state == ONGOING);
    
    close(socket);
    free(current_game);
    game_instances[game_number] = NULL;
    printf("[INFO] GAME N%d TERMINATED\n", game_number);

    return NULL;
}

/*####################################################### thread_func_acceptgames ######################################################*/

void* thread_func_acceptgames()
{
    coms_t buffer_stream;
    int sd_stream; //socket descriptor do servidor para stream
    int s; //client specific socket descriptor
    int new_sock, max_sd, request_made, i;
    struct sockaddr_un streamsv_addr;
    socklen_t streamsv_addrlen;
    fd_set rfds; //set dos descriptors das sockets
    int client_sockets_fd[NJMAX] = {0}; //array com socket descriptors de todos os clientes
    
    //criação da socket stream
    if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ) 
    {
        perror("[ERRO] Não foi possível criar socket stream");
        exit(-1);
    }
  
    //inicializar socket STREAM
    streamsv_addr.sun_family = AF_UNIX;
    memset(streamsv_addr.sun_path, 0, sizeof(streamsv_addr.sun_path));
    strcpy(streamsv_addr.sun_path, JMMSERVSS); //definir path para a socket (vai ficar no /tmp/...)
    streamsv_addrlen = sizeof(streamsv_addr.sun_family) + strlen(streamsv_addr.sun_path);
  
    //remover ficheiro socket caso este já tenha sido criado previamente 
    unlink(JMMSERVSS);
  
    //ligar server à socket (bind)
    if(bind(sd_stream, (struct sockaddr *) &streamsv_addr, streamsv_addrlen) < 0 ) 
    {
        perror("[ERRO] Não consegui dar bind da socket stream"); 
        exit(-1);
    }
  
  
    //socket stream vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if(listen(sd_stream, NJMAX) < 0 ) 
    {
        perror("[ERRO] Listen deu erro"); 
        exit(-1);
    }
  
    printf("[INFO] À espera de receber conexões dos clientes...\n");

    while (true)
    {
        client_addrlen = sizeof(client_addr);
        if((new_sock = accept(sd_stream, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) 
        {
            perror("[ERRO] Accept deu erro");
            exit(-1);
        }

        printf("[AVISO] Novo cliente conectado.\n");

        if(read(new_sock, &buffer_stream, sizeof(buffer_stream)) <= 0) 
        {
            perror("[ERRO] Erro a ler stream");
            exit(-1);
        }

        if (buffer_stream.command == CNJ)
        {
            printf("[INFO] SERVER_STREAM: O jogador %s:%d deseja começar um novo jogo.\n", buffer_stream.arg1.Name, new_sock);

            //criação de novas threads de jogo
            for (int i = 0; i < NJMAX; i++)
            {
                if (game_instances[i] == NULL) //verificar onde há uma posição vazia
                {
                    //começar novo jogo
                    new_game_info game_info = {.game_number = i, .sd=new_sock, .sock_stream = sd_stream, .buffer_s = buffer_stream};
                    if (pthread_create(&thread_gameinstance[i], NULL, thread_func_gameinstance, (void*) &game_info) != 0)
                    {
                        printf("[ERRO] Criação da thread de jogo nº%d falhou\n", i);
                    }
                    else
                    {
                        printf("[INFO] Thread de jogo nº%d criada com sucesso\n", i);
                        //enviar mensagem ao jogador para lhe confirmar que request foi aceite
                        if(write(new_sock, GAME_ACCEPTED, strlen(GAME_ACCEPTED)+1) < 0)    
                        {
                            perror("[ERRO] Erro no envio de stream");
                        }
                    }
                    break; //já criámos thread, podemos prosseguir
                }
                else if (game_instances[i] == NULL && i == NJMAX-1)
                {
                    //informar que servidor está cheio
                    if(write(new_sock, GAME_DENIED, strlen(GAME_DENIED)+1) < 0)    
                    {
                        perror("[ERRO] Erro no envio de stream");
                    }
                    close(new_sock);
                }
                
            }
    
        }
        else printf("[AVISO] Lixo no buffer?\n");
    }

    return NULL;
}


/*####################################################### main ######################################################*/

int main()
{
    int sd_datagram; //socket descriptor do servidor para datagrama
    struct sockaddr_un dgramsv_addr;
    socklen_t dgramsv_addrlen;

    srand(time(NULL));

    //INICIAR THREAD ACEITA JOGOS
    if (pthread_create(&thread_acceptgames, NULL, thread_func_acceptgames, NULL) != 0)
    {
        perror("[ERRO] Criação da thread acceptgames falhou\n");
    }
    else printf("[INFO] ACCEPTGAMES criado\n");

    //criação da socket datagrama
    if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("[ERRO] Não foi possível criar socket datagrama"); 
        exit(-1);
    }

    //inicializar socket DATAGRAM
    dgramsv_addr.sun_family = AF_UNIX;
    memset(dgramsv_addr.sun_path, 0, sizeof(dgramsv_addr.sun_path));
    strcpy(dgramsv_addr.sun_path, JMMSERVSD);
    dgramsv_addrlen = sizeof(dgramsv_addr.sun_family) + strlen(dgramsv_addr.sun_path);

    //remover ficheiro socket caso este já tenha sido criado previamente
    unlink(JMMSERVSD);

    //ligar server à socket (bind)
    if(bind(sd_datagram, (struct sockaddr *) &dgramsv_addr, dgramsv_addrlen) < 0 ) 
    {
        perror("[ERRO] Não consegui dar bind da socket datagrama"); 
        exit(-1);
    }

    //ler datagramas
    while (true)
    {
        client_addrlen = sizeof(client_addr);
        if(recvfrom(sd_datagram, &buffer_dgram, sizeof(buffer_dgram), 0, (struct sockaddr *) &client_addr, &client_addrlen) < 0)
        {
            perror("[ERRO] Erro na recepção de datagrama");
        }
        else
        {
            datagram_handler(sd_datagram, client_addr, client_addrlen); //processar request
        }
    }
    
    return 0;
}