#pragma pack(1)
#include "mastermind.h"
#define MSG "SERVER IS ALIVE AND WELL"

//terei de verificar se de facto é necessário que isto seja global
coms_t buffer_stream, buffer_dgram; //posso usar só 1 buffer com mutex I think mas acho que ia tornar tudo mais lento sem necessidade e o prof não especifica
pthread_t thread_middleman;
pthread_t thread_gameinstance[NJMAX]; //acho que isto pode ser local ao middleman
game_t* game_instances[NJMAX] = {0}; //maybe pode ser removido, confirmar
sem_t sem_mm, sem_handler, sem_create, sem1, sem2, sem3, sem4;
int sd_global = 0;
struct sockaddr_un addr;
socklen_t addrlen;


//inicialização do jogo
void create_new_game(game_t* game, int game_number)
{
    game->log.nd = buffer_stream.arg2.n; //dificuldade
    strcpy(game->log.nj, buffer_stream.arg1.Name); //nome do jogador
    game->log.nt = 0;
    game->log.ti = time(NULL);
    game->log.tf = NO_TIME_REGISTERED; //inicializei com alguma coisa para conseguir perceber se der problemas
    game->nt_max = 3;
    if (buffer_stream.arg2.n == 1)
    {
        game->n_char = 3;
        //falta adicionar aleatoriedade
        strcpy(game->correct_sequence, "AAA"); //inicializa o código secreto
    }
    else if (buffer_stream.arg2.n == 2)
    {
        game->n_char = 5;
        //falta adicionar aleatoriedade
        strcpy(game->correct_sequence, "AAAAA"); //inicializa o código secreto
    }
    
    //game->player_move = "0000"; desnecessário (provavelmente), also tem de ser com strcpy

    game->np = 0;
    game->nb = 0;
    game->game_state = ONGOING;

    //inicializar novas vars que criei
    game->sd = sd_global;

    printf("[INFO] Init do jogo funcionou\n");

    return;
}

void* thread_func_gameinstance(void* arg)
{   
    int game_number = *(int *) arg;
    game_t* current_game = NULL;
    game_state_t state = ONGOING;

    current_game = malloc(sizeof(game_t));
    if (current_game == NULL) 
    {
        perror("[ERRO] Falha a alocar estrutura do jogo");
        exit(-1);
    }

    game_instances[game_number] = current_game;

    create_new_game(current_game, game_number);
    printf("[INFO] Jogo nº%d foi iniciado\n", game_number);
    sem_post(&sem_create);

    while (state == ONGOING)
    {
        switch (game_number)
        {
            case 0:
                sem_wait(&sem1);
                break;
            
            case 1:
                sem_wait(&sem2);
                break;

            case 2:
                sem_wait(&sem3);
                break;

            case 3:
                sem_wait(&sem4);
                break;

            default:
                printf("[ERRO] Game number inválido: %d\n", game_number);
                exit(-1);
        }
        strcpy(current_game->player_move, buffer_stream.arg1.move);
        printf("Move: %s \n Correct move: %s\n", current_game->player_move, current_game->correct_sequence);
        state = analise_move(current_game);
        printf("Gamestate = %d\n", state);
        sem_post(&sem_handler);
    }

    free(current_game);
    return NULL;
}

void* thread_func_middleman()
{
    while (true)
    {
        sem_wait(&sem_mm);
        //criação de novos jogos
        for (int i = 0; i < NJMAX; i++)
        {
            if (thread_gameinstance[i] == 0) //verificar onde há uma posição vazia
            {
                //começar novo jogo
                if (pthread_create(&thread_gameinstance[i], NULL, thread_func_gameinstance, (void*) &i) != 0)
                {
                    printf("[ERRO] Criação da thread de jogo nº%d falhou\n", i);
                }
                else
                {
                    printf("[INFO] Thread de jogo nº%d criada com sucesso\n", i);
                }
                break; //já criámos thread, podemos prosseguir
            }
        }
    }
}

//talvez dê para comprimir writes da função 
void stream_handler(int socket_descriptor)
{

    switch (buffer_stream.command)
    {
        case CNJ:
            printf("[INFO] SERVER_STREAM: O jogador %s:%d deseja começar um novo jogo.", buffer_stream.arg1.Name, socket_descriptor);
            sd_global = socket_descriptor;
            
            //ATIVAR MIDDLEMAN
            sem_post(&sem_mm);
            sem_wait(&sem_create);
            //enviar mensagem ao jogador para lhe confirmar que request foi aceite
            if(write(socket_descriptor, GAME_ACCEPTED, strlen(GAME_ACCEPTED)+1) < 0)    
            {
                perror("[ERRO] Erro no envio de stream");
            }
            break;

        case JG:
            //enviar jogada associada ao socket de onde foi recebida stream para a thread de jogo adequada
            int game_number = -1;
            char result[strlen(MOVE_REGISTERED)+8+1];

            for (int i = 0; i < NJMAX; ++i)
            {
                if ((game_instances[i] != NULL) && (game_instances[i]->sd == socket_descriptor))
                {
                    game_number = i;
                    break;
                }
                else printf("Não é o game number nº%d\n", i);
            }

            switch (game_number)
            {
                case 0:
                    sem_post(&sem1);
                    break;
                
                case 1:
                    sem_post(&sem2);
                    break;

                case 2:
                    sem_post(&sem3);
                    break;

                case 3:
                    sem_post(&sem4);
                    break;

                default:
                    printf("[ERRO] Game number inválido: %d\n", game_number);
                    exit(-1);
            }

            printf("[INFO] SERVER_STREAM: O jogador com descriptor %d deseja fazer a seguinte jogada: %s\n", socket_descriptor, buffer_stream.arg1.move);

            //enviar mensagem ao jogador para lhe mostrar resultado da jogada
            sem_wait(&sem_handler);
            sprintf(result, "nb:%d np:%d", game_instances[game_number]->nb, game_instances[game_number]->np);
            printf("Resultado da jogada: %s\n", result);
            if(write(socket_descriptor, result, strlen(result)+1) < 0)    
            {
                perror("[ERRO] Erro no envio de stream");
            }
            break;
        
        //default:
<<<<<<< Updated upstream
<<<<<<< Updated upstream
        //    printf("[ERRO] Lixo no buffer stream.\n");
        //    break;
=======
          //  printf("[ERRO] Lixo no buffer stream.\n");
            //break;
>>>>>>> Stashed changes
=======
        //    printf("[ERRO] Lixo no buffer stream.\n");
        //    break;
>>>>>>> Stashed changes
    }

    return;
}

void datagram_handler(int socket_descriptor, struct sockaddr_un client_addr, socklen_t client_addrlen)
{
    //printf("[INFO] SERVER_DATAGRAMA: Recebi\"%s\" do cliente \"%s\" \n", buffer_dgram, client_addr.sun_path); //mostrar o que foi recebido
    //enviar algo para o cliente 
    if(sendto(socket_descriptor, MSG, strlen(MSG)+1, 0, (struct sockaddr*) &client_addr, client_addrlen) < 0)
    {
        perror("[ERRO] Erro no envio de datagrama");
    }
}

//esta função está uma macacada enorme, vou ter de passar isto para várias funções
void communications()
{
    int sd_stream; //socket descriptor do servidor para stream
    int sd_datagram; //socket descriptor do servidor para datagrama
    int s; //client specific socket descriptor
    int new_sock, max_sd, request_made, i;
    struct sockaddr_un streamsv_addr, dgramsv_addr;
    socklen_t streamsv_addrlen, dgramsv_addrlen;
    struct sockaddr_un client_addr;
    socklen_t client_addrlen; //não será igual ao server_addrlen? (acho que posso eliminar esta variável e passar o server_addrlen para addrlen)
    fd_set rfds; //set dos descriptors das sockets
    int client_sockets_fd[NJMAX] = {0}; //array com socket descriptors de todos os clientes
    
    //criação das sockets do servidor
    if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ) 
    {
        perror("[ERRO] Não foi possível criar socket stream");
        exit(-1);
    }
    if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("[ERRO] Não foi possível criar socket datagrama"); 
        exit(-1);
    }
  
    //inicialização da estrutura para associar (bind) server à socket
  
    //inicializar socket STREAM
    streamsv_addr.sun_family = AF_UNIX;
    memset(streamsv_addr.sun_path, 0, sizeof(streamsv_addr.sun_path));
    strcpy(streamsv_addr.sun_path, JMMSERVSS); //definir path para a socket (vai ficar no /tmp/...)
    streamsv_addrlen = sizeof(streamsv_addr.sun_family) + strlen(streamsv_addr.sun_path);
  
    //inicializar socket DATAGRAM
    dgramsv_addr.sun_family = AF_UNIX;
    memset(dgramsv_addr.sun_path, 0, sizeof(dgramsv_addr.sun_path));
    strcpy(dgramsv_addr.sun_path, JMMSERVSD);
    dgramsv_addrlen = sizeof(dgramsv_addr.sun_family) + strlen(dgramsv_addr.sun_path);
  
    //remover ficheiro socket caso este já tenha sido criado previamente
    unlink(JMMSERVSD); 
    unlink(JMMSERVSS);
  
    //ligar server às socket (bind)
    if(bind(sd_stream, (struct sockaddr *) &streamsv_addr, streamsv_addrlen) < 0 ) 
    {
        perror("[ERRO] Não consegui dar bind da socket stream"); 
        exit(-1);
    }
    if(bind(sd_datagram, (struct sockaddr *) &dgramsv_addr, dgramsv_addrlen) < 0 ) 
    {
        perror("[ERRO] Não consegui dar bind da socket datagrama"); 
        exit(-1);
    }
  
  
    //socket stream vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if(listen(sd_stream, NJMAX) < 0 ) 
    {
        perror("[ERRO] Listen deu erro"); 
        exit(-1);
    }
  
    printf("[INFO] À espera de receber conexões dos clientes...\n");
  
    //receber mensagens dos clientes
    while(true)
    {
        FD_ZERO(&rfds); //limpar set
        FD_SET(sd_stream, &rfds); //adicionar socket que está listening ao set (para select conseguir ver se temos novos clientes)
        FD_SET(sd_datagram, &rfds);
        max_sd = (sd_stream > sd_datagram) ? sd_stream : sd_datagram; //temos de guardar esta informação (estava na man page, não sei qual é o propósito)
  
        //adicionar sockets dos clientes ao set
        for(i = 0; i < NJMAX; ++i) 
        {
            if(client_sockets_fd[i] > 0) //adicionar ao set se file descriptor existe (>0)
            {
                FD_SET(client_sockets_fd[i], &rfds);
                if(client_sockets_fd[i] > max_sd) max_sd = client_sockets_fd[i]; //atualizar qual é o sd mais recente (valor maior)
            }
        }
  
        //esperar por pedido de conexão (bloqueia)
        printf("[AVISO] Estou no select\n");
        if((request_made = select(max_sd + 1, &rfds, NULL, NULL, NULL)) < 0 /*&& errno != EINTR*/)
        {
            perror("[ERRO] Select deu erro");
        }
  
        //existem pedidos, vamos aceitar conexão
        if(FD_ISSET(sd_stream, &rfds)) 
        {
            client_addrlen = sizeof(client_addr);
            if((new_sock = accept(sd_stream, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) 
            {
                perror("[ERRO] Accept deu erro");
                exit(-1);
            }
  
            printf("[AVISO] Novo cliente conectado.\n");
  
            // Adicionar o novo cliente ao array de sockets
            for(i = 0; i < NJMAX; ++i) 
            {
                if(client_sockets_fd[i] == 0) //procurar "espaço vazio" no array
                {
                    client_sockets_fd[i] = new_sock;
                    printf("[INFO] Socket guardada no indice: %d\n", i);
                    break; //se já encontrei não vale a pena continuar a iterar
                }
            }
        }
  
        //ler stream dos clientes
        for(i = 0; i < NJMAX; ++i)
        {
            s = client_sockets_fd[i]; //atualizar o cliente que estamos a ler
  
            if(s > 0 && FD_ISSET(s, &rfds))
            {
                //ler mensagem do cliente
                if(read(s, &buffer_stream, sizeof(buffer_stream)) <= 0) 
                {
                    perror("[AVISO] Cliente desconectou-se");
                    close(s);
                    client_sockets_fd[i] = 0;
                    thread_gameinstance[i] = 0;
                }
                else 
                {
                    stream_handler(s); //processar request
                }
            }
        }
  
        //ler datagramas dos clientes
        if(FD_ISSET(sd_datagram, &rfds))
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
        
        
    }
    
    //NAO ESQUECER DE FECHAR SOCKET E DAR UNLINK
    close(sd_stream);
    close(sd_datagram);
    /*close(s);*/ //aqui já não é ncessário fazer porque fizemos no loop sempre que cliente se disconecta (altough o que é que acontece se client crashar?)
    unlink(JMMSERVSD);
    unlink(JMMSERVSS);
}

game_state_t analise_move(game_t *game_pt)
{ // MAX_SEQUENCE_SIZE

    unsigned short int i, j, np = 0, nb = 0;
    unsigned short int used_secret[MAX_SEQUENCE_SIZE] = {false};
    unsigned short int used_guess[MAX_SEQUENCE_SIZE] = {false};

    // incrementar o nº de jogadas
    game_pt->log.nt++;

    // procurar por letras certas no sítio certo
    for (int i = 0; i < game_pt->n_char; ++i)
    {
        if (game_pt->correct_sequence[i] == game_pt->player_move[i])
        {
            np++;
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
        game_pt->log.tf = time(NULL);
        return game_pt->game_state = PLAYER_WIN;
    }
    else if (game_pt->nt_max == ++(game_pt->log.nt))
    {
        // jogador sem mais tentativas, perdeu
        game_pt->log.tf = time(NULL);
        return game_pt->game_state = PLAYER_LOST;
    }

    // jogo continua
    return ONGOING;
}

int main() {

    //criar semáforo para middleman
    if(sem_init(&sem_mm, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo mm falhou");
    }
    if(sem_init(&sem_handler, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo handler falhou");
    }
    if(sem_init(&sem_create, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo mm falhou");
    }
    if(sem_init(&sem1, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo 1 falhou");
    }
    if(sem_init(&sem2, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo 2 falhou");
    }
    if(sem_init(&sem3, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo 3 falhou");
    }
    if(sem_init(&sem4, 0, 0) != 0)
    {
        perror("[ERRO] Criação do semáforo 4 falhou");
    }

    //criar middleman
    if (pthread_create(&thread_middleman, NULL, thread_func_middleman, NULL) != 0)
    {
        perror("[ERRO] Criação da thread middleman falhou\n");
    }
    else printf("[INFO] MIDDLEMAN criado\n");

    communications();

    //sair ordeiramente
    sem_destroy(&sem_mm);
    sem_destroy(&sem_handler);
    sem_destroy(&sem1);
    sem_destroy(&sem2);
    sem_destroy(&sem3);
    sem_destroy(&sem4);
    
    return 0;
}