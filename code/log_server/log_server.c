#include "log_server.h"
#include <unistd.h>

//?? É preferivel deixar o ficheiro aberto e ir sincronizando ou ->**abrir e fechar o ficheiro várias vezes?**<-

#define FILE "FICHEIRO.DAT"

// log_init();
// JMMLOGSD
// JMMLOGQ <-
// add_game()
// get_tab_n()
// del_tab_n() -> a função de delere devia ser a primeira a ser chamada para garantir uma tabela vazia




/***************************** global variables *******************************/
struct mq_attr ma; //!! por local e static
bool STATUS_ON = true;
pthread_mutex_t file_mux;

/*
struct 	command_d {
  void  (*cmd_fnct)(int, char**);
  char* cmd_name;
  char* cmd_help;
} const commands[] = {
  {get_tab_n, "ltc","<arg n> listar tabela(s) classificação nível n"},
  {cmd_rtc, "rtc","<arg n> reinicializar tabela(s) classificação nível n (0-todos)"},
  {cmd_trh, "trh","terminar processo de registo histórico (JMMlog)"}
}; */



/***************************** open_file *******************************/
void open_file(int* mfd_p, log_tabs_t** tabel_p) {
  /* abrir / criar ficheiro */
  if ((*mfd_p = open(FILE, O_RDWR | O_CREAT, 0666)) < 0) {
    perror("Erro a criar ficheiro");
    exit(-1);
  }
  else {
    /* definir tamanho do ficheiro */
    if (ftruncate(*mfd_p, MSIZE) < 0) {
      perror("Erro no ftruncate");
      exit(-1);
    }
  }
  /* mapear ficheiro */
  if ((*tabel_p = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, *mfd_p, 0)) < (log_tabs_t*)0) {
    perror("Erro em mmap");
    exit(-1);
  }
}

/***************************** termination_handler *******************************/
void termination_handler(int signum)
{
  //exit here or notify other processes to end
  printf("-------------termination_handler: starting exit----------\n");
  STATUS_ON = false;
  return;
}

/***************************** get_tab_n *******************************/
void get_tab_n(log_single_tab_t* single_tab, int diff) {
  //! pthread_mutex_lock(&file_mux);  //entering critical secttion -> fazer na função acima
  int mfd;
  log_tabs_t* tabel;

  open_file(&mfd, &tabel);

  switch (diff)
  {
  case DIFF_1:
    for (int i = 0; i < tabel->tb1_n_games; i++) {
      single_tab->tb[i] = tabel->tb1[i];
    }
    single_tab->tb_diff = diff;
    single_tab->tb_n_games = tabel->tb1_n_games;
    break;

  case DIFF_2:
    for (int i = 0; i < tabel->tb2_n_games; i++) {
      single_tab->tb[i] = tabel->tb2[i];
    }
    single_tab->tb_diff = diff;
    single_tab->tb_n_games = tabel->tb2_n_games;
    break;

  default:
    printf("get_tab_n: difficuldade inválida\n");
    break;
  }


}

/***************************** del_tab_n *******************************/
void del_tab_n(int diff) {

  int mfd;
  log_tabs_t* tabel;

  open_file(&mfd, &tabel);

  switch (diff) {
  case DIFF_ALL:
    tabel->tb1_n_games = 0;
    tabel->tb2_n_games = 0;
    break;
  case DIFF_1:
    tabel->tb1_n_games = 0;
    break;
  case DIFF_2:
    tabel->tb2_n_games = 0;
    break;

  default:
    printf("del_tab_n: difficuldade inválida\n");
    break;
  }
}



/***************************** queue_handler *******************************/
void* queue_handler(void* pi)
{
  printf("queue_handler: begin thread\n");
  int mqids;
  rjg_t game_save;

  // variáveis para abrir ficheiro 
  int mfd;
  log_tabs_t* tabel;

  struct timespec tm;

  // definir termination signal handler 
  if (signal(SIGTERM, termination_handler) == SIG_ERR) {
    printf("error setting SIGTERM signal\n");
  }
  if (signal(SIGINT, termination_handler) == SIG_ERR) {
    printf("error setting SIGTERM signal\n");
  }

  // open queue
  printf("queue_handler: abrir queue\n");
  ma.mq_flags = 0;
  ma.mq_maxmsg = 3;
  ma.mq_msgsize = sizeof(game_save);
  if ((mqids = mq_open(JMMLOGQ, O_RDWR | O_CREAT, 0666, &ma)) < 0) {
    perror("queue_handler: Erro a criar queue servidor\n");
    exit(-1); // TODO exit? ou return? -> (exit mata o processo todo, not a clean exit)-> implement clean exit, with returns I guess
  }
  printf("queue_handler: acabei de abrir queue\n");


  // continually parse messages
  while (STATUS_ON) {
    // get message/game log
    printf("\nqueue_handler: get message/game log\n");
    //?clock_gettime(CLOCK_REALTIME, &tm);
    //?tm.tv_sec += 10;  // Set for 1 seconds
    //?if (mq_timedreceive(mqids, (char*)&game_save, sizeof(game_save), NULL, &tm) < 0) {
    if (mq_receive(mqids, (char*)&game_save, sizeof(game_save), NULL) < 0) {
      perror("queue_handler: erro a receber mensagem ou timeout");
    }

    pthread_mutex_lock(&file_mux);  //entering critical secttion

    // abrir ficheiro com mmap
    open_file(&mfd, &tabel);

    /* guardar o jogo na memória */
    printf("queue_handler: guardar o jogo na memória\n");
    switch (game_save.nd)
    {
    case DIFF_1:
      if (tabel->tb1_n_games >= TOPN) {
        printf("queue_handler: tabela da dificuldade 1 cheia\n");
      }
      else {
        tabel->tb1[tabel->tb1_n_games] = game_save; //guardar cópia
        tabel->tb1_n_games++;
      }
      break;

    case DIFF_2:
      if (tabel->tb2_n_games >= TOPN) {
        printf("queue_handler: tabela da dificuldade 2 cheia\n");
      }
      else {
        tabel->tb2[tabel->tb2_n_games] = game_save; //guardar cópia
        tabel->tb2_n_games++;
      }
      break;

    default:
      printf("queue_handler: difficuldade inválida\n");
      break;
    }
    printf("queue_handler: acabei de guardar jogo na memória\n");
    // fechar ficheiro
    munmap(tabel, MSIZE);
    close(mfd);
    pthread_mutex_unlock(&file_mux);  //exiting critical secttion
  }

  // clean exit process
  if (mq_unlink(JMMLOGQ) < 0) {
    perror("queue_handler: Erro a eliminar queue");
  }
}
/******************************************************************/
/***************************** main *******************************/
int main()
{
  pthread_t thread_queue_handler;

  // variáveis do socket datagrama
  int socket_d;
  struct sockaddr_un my_addr;
  socklen_t addrlen;
  struct sockaddr_un from;
  socklen_t fromlen;
  msg_to_JMMlog msg_recieved;

  log_single_tab_t msg_tab_send;


  printf("A começar JMMlog (PID=%d)\n", getpid());

  if (pthread_mutex_init(&file_mux, NULL) != 0) {
    printf("Erro a inicializar mutex\n");
    return -1;
  }

  //* começar thread_queue_handler -- comunicação com JMMserv
  printf("main: criar thread Queue Handler\n");
  if (pthread_create(&thread_queue_handler, NULL, queue_handler, NULL) != 0) {
    printf("Erro a criar thread=Queue Handler\n");
  }


  //* começar ligação socket datagrama -- comunicação com JMMapl
  if ((socket_d = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0)
  {
    perror("Erro a criar socket");
    exit(-1);
  }
  my_addr.sun_family = AF_UNIX;
  memset(my_addr.sun_path, 0, sizeof(my_addr.sun_path));
  strcpy(my_addr.sun_path, JMMLOGSD);
  addrlen = sizeof(my_addr.sun_family) + strlen(my_addr.sun_path);

  if (bind(socket_d, (struct sockaddr*)&my_addr, addrlen) < 0)
  {
    perror("Erro no bind");
    exit(-1);
  }

  while (STATUS_ON) { // receber datagramas and parse
    fromlen = sizeof(from);
    if (recvfrom(socket_d, &msg_recieved, sizeof(msg_recieved), 0, (struct sockaddr*)&from, &fromlen) < 0)    perror("Erro no recvfrom");
    else {
      printf("SERV: Recebi: cmd=%s n=%i Path: %s\n", msg_recieved.cmd, msg_recieved.arg_n, from.sun_path);
     
      if (strcmp(msg_recieved.cmd, "ltc") == 0) { //* - listar tabela(s) classificação nível n (0-todos)
        printf("listar tabela(s) classificação nível n=\n", msg_recieved.arg_n);
        pthread_mutex_lock(&file_mux);  //entra na zona critical
        switch (msg_recieved.arg_n) {
        case DIFF_ALL:
          get_tab_n(&msg_tab_send, DIFF_1);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          get_tab_n(&msg_tab_send, DIFF_2);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        case DIFF_1:
          get_tab_n(&msg_tab_send, msg_recieved.arg_n);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        case DIFF_2:
          get_tab_n(&msg_tab_send, msg_recieved.arg_n);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        default:
          printf("main: difficuldade inválida\n");
          break;
        }
        pthread_mutex_unlock(&file_mux);  //sair da zona crítica
      }

      else if (strcmp(msg_recieved.cmd, "rtc") == 0) { //*- reinicializar tabela(s) classificação nível n (0-todos)
        printf("reinicializar tabela(s) classificação nível n=\n", msg_recieved.arg_n);
        pthread_mutex_lock(&file_mux);  //entra na zona critical
        del_tab_n(msg_recieved.arg_n);
        pthread_mutex_unlock(&file_mux);  //sair da zona crítica
      }

      else if (strcmp(msg_recieved.cmd, "trh") == 0) { //*- terminar processo de registo histórico (JMMlog)
        printf("terminar processo de registo histórico (JMMlog)\n");
        STATUS_ON = false;
      }
      else {
        perror("comando recebido inválido\n");
      }

    }
  }
  close(socket_d);
  unlink(JMMLOGSD);
  printf("main: terminámos o socket datagram\n");


  pthread_join(thread_queue_handler, NULL); // se a main terminar, o processo termina, e todos as threads associadas também

  printf("MMJ terminado com sucesso\n");
  return 0;
}
