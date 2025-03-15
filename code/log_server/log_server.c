#include "log_server.h"

//?? É preferivel deixar o ficheiro aberto e ir sincronizando ou ->**abrir e fechar o ficheiro várias vezes?**<-

#define FILE "FICHEIRO.DAT"

// log_init();
// JMMLOGSD
// JMMLOGQ <-
// add_game()
// get_tab_n()
// del_tab_n()

/***************************** add_game *******************************/
void add_game()
{

}

/***************************** global variables *******************************/
struct mq_attr ma; //!! por local e static


/***************************** queue_handler *******************************/
void* queue_handler(void* pi)
{
  int mqids;
  rjg_t game_save;

  int i, mfd;
  char* tabel;

  // open queue
  if ((mqids = mq_open(JMMLOGQ, O_RDWR | O_CREAT, 0666, &ma)) < 0) {
    perror("queue_handler: Erro a criar queue servidor");
    exit(-1); // TODO exit? ou return? -> (exit mata o processo todo, not a clean exit)-> implement clean exit, with returns I guess
  }

  // continually parse messages
  while (true) {
    // get message/game log
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

    /* aceder ao ficheiro através da memória */
    for (i = 0; i < MSIZE; i++)
      tabel[i] = 'A';

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
  if (pthread_create(&thread_queue_handler, NULL, queue_handler, NULL) != 0) {
    printf("Erro a criar thread=Queue Handler\n");
  }

  return 0;
}
