/* Exemplo de utilização do "mmap" para mapeamento de um ficheiro 
em memória (ou criação de uma zona de memória partilhada) -- CRA
*/
#include "log_server.h"

 
#define FILE "FICHEIRO.DAT"




int main()
{
  int i, mfd;
  char *tabel;

  printf("Tamanho: %lu\n",sysconf(_SC_PAGE_SIZE));

  if ((mfd=open(FILE, O_RDWR|O_CREAT, 0666 )) < 0) {  /* abrir / criar ficheiro */
    perror("Erro a criar ficheiro");
    exit(-1);
  }
  else {
    if (ftruncate(mfd,  MSIZE) < 0) {                  /* definir tamanho do ficheiro */
      perror("Erro no ftruncate");
      exit(-1);
    }
  }
  /* mapear ficheiro */
  if ((tabel=mmap(NULL, MSIZE, PROT_READ|PROT_WRITE, MAP_SHARED, mfd, 0)) < (log_tabs_t *)0) {
    perror("Erro em mmap");
    exit(-1);
  }

  /* aceder ao ficheiro através da memória */
  for (i=0; i<MSIZE; i++) tabel[i]='A';

  munmap(tabel, MSIZE);
  close(mfd);


  return 0;
}
