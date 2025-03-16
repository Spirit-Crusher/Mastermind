#include "mastermind.h"
#define MSG "SERVER IS ALIVE AND WELL"

//terei de verificar se de facto é necessário que isto seja global
char buffer_stream[100], buffer_dgram[100]; //posso usar só 1 buffer com mutex I think mas acho que ia tornar tudo mais lento sem necessidade e o prof não especifica
pthread_t thread_middleman;
pthread_t thread_gameinstance[NJMAX]; //acho que isto pode ser local ao middleman
game_t* game_instances[NJMAX] = {0};
bool new_game = false;
pthread_mutex_t games_mutex;


//inicialização do jogo
void create_new_game(game_t* game)
{
    strcpy(game->correct_sequence, "AAAA"); //inicializa o nome do jogador
    game->log.nd = buffer_dgram[8]; //dificuldade está na posição 8
    strncpy(game->log.nj, &buffer_dgram[4], 3); //inicializa o nome do jogador
    game->log.nj[3] = '\0'; //adicionar terminação da string
    game->log.nt = 0;
    game->log.ti = time(NULL);
    game->log.tf = NO_TIME_REGISTERED; //inicializei com alguma coisa para conseguir perceber se der problemas
    game->n_char = 4;
    game->nt_max = 3; 
    
    //game->player_move = "0000"; desnecessário (provavelmente), also tem de ser com strcpy

    game->np = 0;
    game->nb = 0;
    game->game_state = ONGOING;

    printf("[INFO] Init funcionou\n");

    return;
}

void* thread_func_gameinstance(void* arg)
{   
    int game_number = *(int *) arg;
    game_t* current_game = NULL;

    current_game = malloc(sizeof(game_t));
    if (current_game == NULL) 
    {
        perror("[ERRO] Falha a alocar estrutura do jogo");
        exit(-1);
    }

    game_instances[game_number] = current_game;

    create_new_game(game_instances[game_number]);
    printf("[INFO] Jogo foi nº%d iniciado\n", game_number);

    while (true)
    {

    }

    //FAZER FREE PLZ
    
}

void* thread_func_middleman()
{
    while (true)
    {
        //criação de novos jogos
        if (new_game)
        {
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

            pthread_mutex_lock(&games_mutex);
            new_game = false;
            pthread_mutex_unlock(&games_mutex);
        }

        //gestão dos jogos já existentes. maybe not. isso pode ser feita pela própria game instance. 
        //nesse caso não preciso de mutex, basta um semáforo que informe o middle man que tem de criar novo jogo

    }
}

//talvez dê para comprimir writes da função 
void stream_handler(int socket_descriptor)
{
    if(!strncmp(buffer_stream, "cnj", 3))
    {
        printf("[INFO] SERVER_STREAM: O jogador com descriptor %d deseja começar um novo jogo: %s\n", socket_descriptor, buffer_stream);
        
        //ATIVAR MIDDLEMAN
        pthread_mutex_lock(&games_mutex);
        new_game = true;
        pthread_mutex_unlock(&games_mutex);

        //enviar mensagem ao jogador para lhe confirmar que request foi aceite
        if(write(socket_descriptor, GAME_ACCEPTED, strlen(GAME_ACCEPTED)+1) < 0)    
        {
            perror("[ERRO] Erro no envio de stream");
        }
    }
    else if(!strncmp(buffer_stream, "jg", 2))
    {
        //enviar jogada associada ao socket de onde foi recebida stream para a thread de jogo adequada

        //semáforos?(um para cada jogo a avisar que jogada está pronta ou assim)


        printf("[INFO] SERVER_STREAM: O jogador com descriptor %d deseja fazer a seguinte jogada: %s\n", socket_descriptor, buffer_stream);
        //enviar mensagem ao jogador para lhe confirmar que request foi aceite
        if(write(socket_descriptor, MOVE_REGISTERED, strlen(MOVE_REGISTERED)+1) < 0)    
        {
            perror("[ERRO] Erro no envio de stream");
        }
    }
    else
    {
        printf("[ERRO] Lixo no buffer stream\n");
    }
}

void datagram_handler(int socket_descriptor, struct sockaddr_un client_addr, socklen_t client_addrlen)
{
    printf("[INFO] SERVER_DATAGRAMA: Recebi\"%s\" do cliente \"%s\" \n", buffer_dgram, client_addr.sun_path); //mostrar o que foi recebido
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
  
  
    unlink(JMMSERVSD); //remover ficheiro socket caso este já tenha sido criado previamente
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
  
            if(s > 0)
            {
                //ler mensagem do cliente
                if(read(s, buffer_stream, sizeof(buffer_stream)) <= 0) 
                {
                    perror("[AVISO] Cliente desconectou-se");
                    close(s);
                    client_sockets_fd[i] = 0;
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
            if(recvfrom(sd_datagram, buffer_dgram, sizeof(buffer_dgram), 0, (struct sockaddr *) &client_addr, &client_addrlen) < 0)
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


int main() {

    //criar mutex
    if(pthread_mutex_init(&games_mutex, NULL) != 0)
    {
        perror("[ERRO] Criação do mutex falhou");
    }

    //criar middleman
    if (pthread_create(&thread_middleman, NULL, thread_func_middleman, NULL) != 0)
    {
        perror("[ERRO] Criação da thread middleman falhou\n");
    }
    else printf("[INFO] MIDDLEMAN criado\n");

    communications();

    return 0;
}