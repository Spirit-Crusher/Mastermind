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


/***************************** termination_handler *******************************/
void termination_handler(int signum)
{
  //exit here or notify other processes to end
  printf("termination_handler: starting exit\n");
  STATUS_ON = false;
  return;
}

/***************************** get_tab_n *******************************/
//get_tab_n()

/***************************** test_print_memory *******************************/
void test_print_memory(){
  rjg_t game_save;

  int mfd;
  log_tabs_t* tabel;

  /* abrir / criar ficheiro */
  if ((mfd = open(FILE, O_RDWR | O_CREAT, 0666)) < 0) {
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
  if ((tabel = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0)) < (log_tabs_t*)0) {
    perror("Erro em mmap");
    exit(-1);
  }

  /* mostrar os jogos na memória */
  printf("test_print_memory: mostrar os jogos na memória\n");

  printf("nº de jogos na tabela 1: %u\n", tabel->tb1_n_games);
  for (int i = 0; i < tabel->tb1_n_games; i++){
    printf("jogo nº%i: nd=%i nome=%s\n", i,tabel->tb1[i].nd, tabel->tb1[i].nj);
  }
  printf("\nnº de jogos na tabela 2: %u\n", tabel->tb1_n_games);
  for (int i = 0; i < tabel->tb1_n_games; i++){
    printf("jogo nº%i: nd=%i nome=%s\n", i,tabel->tb1[i].nd, tabel->tb1[i].nj);
  }

  // fechar ficheiro
  munmap(tabel, MSIZE);
  close(mfd);
}





/***************************** queue_handler *******************************/
void* queue_handler(void* pi)
{
  printf("queue_handler: begin thread\n");
  int mqids;
  rjg_t game_save;

  int mfd;
  log_tabs_t* tabel;

  // definir termination signal handler 
  if (signal(SIGTERM, termination_handler) == SIG_ERR) {
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
    printf("queue_handler: get message/game log\n");
    if (mq_receive(mqids, (char*)&game_save, sizeof(game_save), NULL) < 0) {
      perror("queue_handler: erro a receber mensagem");
    }

    /* abrir / criar ficheiro */
    if ((mfd = open(FILE, O_RDWR | O_CREAT, 0666)) < 0) {
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
    if ((tabel = mmap(NULL, MSIZE, PROT_READ | PROT_WRITE, MAP_SHARED, mfd, 0)) < (log_tabs_t*)0) {
      perror("Erro em mmap");
      exit(-1);
    }

    /* guardar o jogo na memória */
    printf("queue_handler: guardar o jogo na memória\n");
    switch (game_save.nd)
    {
    case DIFF_1:
      if (tabel->tb1_n_games == TOPN) {
        printf("queue_handler: tabela da dificuldade 1 cheia\n");
      }
      tabel->tb1[tabel->tb1_n_games] = game_save; //guardar cópia
      break;

    case DIFF_2:
      if (tabel->tb2_n_games == TOPN) {
        printf("queue_handler: tabela da dificuldade 2 cheia\n");
      }
      tabel->tb2[tabel->tb2_n_games] = game_save; //guardar cópia
      break;

    default:
      printf("queue_handler: difficuldade inválida\n");
      break;
    }
    printf("queue_handler: guardar o jogo na memória\n");
    // fechar ficheiro
    munmap(tabel, MSIZE);
    close(mfd);


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
  // printf("Tamanho: %lu\n",sysconf(_SC_PAGE_SIZE));

  printf("main: criar thread Queue Handler\n");
  //if (pthread_create(&thread_queue_handler, NULL, queue_handler, NULL) != 0) {
  //  printf("Erro a criar thread=Queue Handler\n");
  //}

  pthread_join(thread_queue_handler, NULL); // se a main terminar, o processo termina, e todos as threads associadas também
  return 0;
}
