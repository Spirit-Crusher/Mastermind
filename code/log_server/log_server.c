#include "log_server.h"
#include <unistd.h>

/***************************** global variables *******************************/
bool STATUS_ON = true;  // variável que avisa o programa se deve terminar ou não
pthread_mutex_t file_mux; // mutex para acesso ao ficheiro com logs de jogo

//variáveis da gestão do ficheiro
int mfd;
log_tabs_t* tabel_pt;
//variáveis do socket
int socket_d;

/***************************** termination_handler *******************************/
void termination_handler()
{
  //exit here or notify other processes to end
  printf("-------------termination_handler: starting exit----------\n");

  pthread_mutex_lock(&file_mux);  //entra na zona critical (só fechar se ninguém estiver a usar)
  // fechar ficheiro de dados
  munmap(tabel_pt, MSIZE);
  close(mfd);
  printf("termination_handler: Ficheiro de dados Fechado\n");

  //fechar socket
  close(socket_d);
  unlink(JMMLOGSD);
  printf("termination_handler: Terminámos o socket datagram\n");

  //fechar queue
  if (mq_unlink(JMMLOGQ) < 0) {
    perror("termination_handler: Erro a eliminar queue");
  }
  printf("termination_handler: queue eliminada de forma limpa\n");

  exit(0);
  return;
}


/******************************************************************/
/***************************** main *******************************/
int main()
{
  pthread_t thread_queue_handler;

  // variáveis do socket datagrama
  struct sockaddr_un my_addr;
  socklen_t addrlen;
  struct sockaddr_un from;
  socklen_t fromlen;
  coms_t msg_recieved;

  log_single_tab_t msg_tab_send;

  printf("A começar JMMlog (PID=%d)\n", getpid());

  // definir termination signal handler 
  if (signal(SIGTERM, termination_handler) == SIG_ERR) {
    perror("error setting SIGTERM signal\n");
  }
  if (signal(SIGINT, termination_handler) == SIG_ERR) {
    perror("error setting SIGTERM signal\n");
  }

  if (pthread_mutex_init(&file_mux, NULL) != 0) {
    printf("Erro a inicializar mutex\n");
    return -1;
  }


  // abrir ficheiro com mmap e mapiá-lo a uma variável
  open_file(&mfd, &tabel_pt);

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

  while (STATUS_ON) { // receber datagramas e enviar respostas
    fromlen = sizeof(from);
    if (recvfrom(socket_d, &msg_recieved, sizeof(msg_recieved), 0, (struct sockaddr*)&from, &fromlen) < 0)    perror("Erro no recvfrom");
    else {
      printf("SERV: Recebi: cmd=%i n=%d Path: %s\n", msg_recieved.command, msg_recieved.arg1.n, from.sun_path);

      //************** - ltc: listar tabela(s) classificação nível n (0-todos)
      if (msg_recieved.command == LTC) {
        printf("listar tabela(s) classificação nível n=%i\n", msg_recieved.arg1.n);

        pthread_mutex_lock(&file_mux);  //entra na zona critical
        switch (msg_recieved.arg1.n) {
        case DIFF_ALL:
          get_tab_n(&msg_tab_send, DIFF_1);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          get_tab_n(&msg_tab_send, DIFF_2);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        case DIFF_1:
          get_tab_n(&msg_tab_send, msg_recieved.arg1.n);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        case DIFF_2:
          get_tab_n(&msg_tab_send, msg_recieved.arg1.n);
          if (sendto(socket_d, &msg_tab_send, sizeof(log_single_tab_t), 0, (struct sockaddr*)&from, fromlen) < 0) perror("Erro no sendto\n");
          break;
        default:
          printf("main: difficuldade inválida\n");
          break;
        }
        pthread_mutex_unlock(&file_mux);  //sair da zona crítica
      }
      //************** - rtc: reinicializar tabela(s) classificação nível n (0-todos)
      else if (msg_recieved.command == RTC) {
        printf("reinicializar tabela(s) classificação nível n=%i\n", msg_recieved.arg1.n);
        pthread_mutex_lock(&file_mux);  //entra na zona critical
        del_tab_n(msg_recieved.arg1.n);
        pthread_mutex_unlock(&file_mux);  //sair da zona crítica
      }

      //************** - trh: terminar processo de registo histórico (JMMlog)
      else if (msg_recieved.command == TRH) {
        printf("terminar processo de registo histórico (JMMlog)\n");
        termination_handler();
      }

      //************** - comando não válido
      else {
        perror("comando recebido inválido\n");
      }

    }
  }

  pthread_join(thread_queue_handler, NULL); // se a main terminar, o processo termina, e todos as threads associadas também

  printf("JMMlog todas as threads terminadas - Clena Exit\n");
  return 0;
}

/***************************************************************************/
/***************************** queue_handler *******************************/
void* queue_handler() {
  printf("queue_handler: começar thread\n");

  //variáveis queue
  int mqids;
  struct mq_attr ma;

  //struct timespec tm;
  rjg_t game_save;

  // abrir queue
  printf("queue_handler: abrir queue\n");
  ma.mq_flags = 0;
  ma.mq_maxmsg = 3;
  ma.mq_msgsize = sizeof(game_save);
  if ((mqids = mq_open(JMMLOGQ, O_RDWR | O_CREAT, 0666, &ma)) < 0) {
    perror("queue_handler: Erro a criar queue servidor\n");
    exit(-1); //? exit? ou return? -> (exit mata o processo todo, not a clean exit)-> implement clean exit, with returns I guess
  }
  printf("queue_handler: acabei de abrir queue\n");

  // receber mensagens da queue e processá-las
  while (STATUS_ON) {
    // receber game log
    printf("\nqueue_handler: pronto receber game log\n");
    if (mq_receive(mqids, (char*)&game_save, sizeof(game_save), NULL) < 0) {
      perror("queue_handler: erro a receber mensagem ou timeout");
    }

    pthread_mutex_lock(&file_mux);  //entering critical secttion
    // guardar o jogo na memória
    printf("queue_handler: guardar o jogo na memória\n");
    insert_sorted_n(tabel_pt, game_save, game_save.nd);
    printf("queue_handler: acabei de guardar jogo na memória\n");
    pthread_mutex_unlock(&file_mux);  //exiting critical secttion
  }

  // clean exit process
  if (mq_unlink(JMMLOGQ) < 0) {
    perror("queue_handler: Erro a eliminar queue");
  }
  printf("queue_handler: queue_handler terminado de forma limpa\n");
  return NULL;
}



/***************************** open_file *******************************/
//void open_file(int* mfd_p, log_tabs_t** tabel_p) {
void open_file() {
  /* abrir / criar ficheiro */
  if ((mfd = open(JMMLOG, O_RDWR | O_CREAT, 0666)) < 0) {
    perror("Erro a criar ficheiro");
    exit(-1);
  }
  else {
    /* definir tamanho do ficheiro */
    if (ftruncate(mfd, MSIZE) < 0) {
      perror("Erro no ftruncate");
      exit(-1);
    }
  }
  /* mapear ficheiro */
  if ((tabel_pt = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0)) < (log_tabs_t*)0) {
    perror("Erro em mmap");
    exit(-1);
  }
  printf("Ficheiro de dados Aberto\n");
}

/***************************** insert_sorted_n *******************************/
void insert_sorted_n(log_tabs_t* log, rjg_t new_game, game_diff_t diff) {
  rjg_t* table;
  int* n_games;

  // Selecionar a tabela correta
  if (diff == 1) {
    table = log->tb1;
    n_games = &log->tb1_n_games;
  }
  else if (diff == 2) {
    table = log->tb2;
    n_games = &log->tb2_n_games;
  }
  else {
    printf("Erro: tabela inválida (use 1 ou 2)\n");
    return;
  }

  if (*n_games >= TOPN) {
    printf("Erro: tabela %d está cheia!\n", diff);
    return;
  }

  int duration_new = new_game.tf - new_game.ti;
  int i = *n_games;

  // Encontrar posição correta para inserir
  while (i > 0) {
    int nt_current = table[i - 1].nt;
    int duration_current = table[i - 1].tf - table[i - 1].ti;

    // Ordem principal: Menos tentativas primeiro
    // Desempate: Menor duração primeiro
    if (nt_current < new_game.nt || (nt_current == new_game.nt && duration_current <= duration_new)) {
      break;
    }

    table[i] = table[i - 1];  // Desloca os elementos para frente
    i--;
  }

  // Insere o novo jogo na posição correta
  table[i] = new_game;
  (*n_games)++;
}


/***************************** get_tab_n *******************************/
void get_tab_n(log_single_tab_t* single_tab, int diff) {

  switch (diff)
  {
  case DIFF_1:
    for (int i = 0; i < tabel_pt->tb1_n_games; i++) {
      single_tab->tb[i] = tabel_pt->tb1[i];
    }
    single_tab->tb_diff = diff;
    single_tab->tb_n_games = tabel_pt->tb1_n_games;
    break;

  case DIFF_2:
    for (int i = 0; i < tabel_pt->tb2_n_games; i++) {
      single_tab->tb[i] = tabel_pt->tb2[i];
    }
    single_tab->tb_diff = diff;
    single_tab->tb_n_games = tabel_pt->tb2_n_games;
    break;

  default:
    printf("get_tab_n: difficuldade inválida (nota: nesta função DIF_ALL não é suportado)\n");
    break;
  }
}


/***************************** del_tab_n *******************************/
void del_tab_n(int diff) {

  switch (diff) {
  case DIFF_ALL:
    tabel_pt->tb1_n_games = 0;
    tabel_pt->tb2_n_games = 0;
    break;
  case DIFF_1:
    tabel_pt->tb1_n_games = 0;
    break;
  case DIFF_2:
    tabel_pt->tb2_n_games = 0;
    break;

  default:
    printf("del_tab_n: difficuldade inválida\n");
    break;
  }
}
