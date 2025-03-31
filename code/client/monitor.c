#include "cliente.h"

/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/ 
const char TitleMsg[] = "\n Application Control Monitor\n";
const char InvalMsg[] = "\n Invalid command!";

struct command_d {
  void  (*cmd_fnct)(int, char**);
  char*	cmd_name;
  char*	cmd_help;
} 

const commands[] = {
  {cmd_sos,  "sos","                       "},
  {cmd_sos,  "help","                      "},
  {cmd_rgr,  "rules","                 listar as regras do jogo"},
  {cmd_sair, "sair","                  sair da aplicação de jogo"},
  {cmd_test, "teste","<arg1> <arg2>    comando de teste"},
  {cmd_cnj, "cnj", "<N> <n>            começar novo jogo (Nome (N) (3 chars) e nível de dificuldade (n)(1/2))"},
  {cmd_jg, "jg", "mplay               jogada (3 ou 5 letras, dependendo do nível"},
  {cmd_clm, "clm","                   consultar limites (número máximo de jogadas, tempo limite)"},
  {cmd_mlm, "mlm","<j> <t>            mudar limites (número máximo de jogadas (j), tempo limite (t) (minutos))"},
  {cmd_cer, "cer","                   consultar estado envio de registos para histórico"},
  {cmd_aer, "aer","                   activar envio de registos para histórico"},
  {cmd_der, "der","                   desactivar envio de registos para histórico"},
  {cmd_tmm, "tmm","                   terminar processo master mind (JMMserv)"},
  {cmd_ltc, "ltc","<n>                listar tabela(s) classificação nível n (0-todos)"},
  {cmd_rtc, "rtc","<n>                reinicializar tabela(s) classificação nível n (0-todos)"},
  {cmd_trh, "trh","                   terminar processo de registo histórico (JMMlog)"}  
};



/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/ 
void cmd_sos (int argc, char **argv){
  unsigned long int i;

  printf("%s\n", TitleMsg);
  for(i = 0; i < NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}


/*-------------------------------------------------------------------------+
| Function: my_getline        (called from monitor) 
+--------------------------------------------------------------------------*/ 
int my_getline(char** argv, int argvsize){
  char *p;
  int argc;
  static char line[MAX_LINE];

  fgets(line, MAX_LINE, stdin);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for(argc = 0,p=line; (*line != '\0') && (argc < argvsize); p = NULL,argc++){
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if(p == NULL) return argc;
  }
  argv[argc] = p;

  return argc;
}


/*-------------------------------------------------------------------------+
| Function: entrance    
+--------------------------------------------------------------------------*/
void entrance(){
  printf("-----------------------------WELCOME TO:---------------------------------      \n");
  printf("\n");
  printf(" __  __           _____ _______ ______ _____  __  __ _____ _   _ _____         \n");
  printf(" __  __           _____ _______ ______ _____  __  __ _____ _   _ _____         \n");  
  printf("|  \\/  |   /\\    / ____|__   __|  ____|  __ \\|  \\/  |_   _| \\ | |  __ \\  \n");
  printf("| \\  / |  /  \\  | (___    | |  | |__  | |__) | \\  / | | | |  \\| | |  | |   \n");                                                                  
  printf("| |\\/| | / /\\ \\  \\___ \\   | |  |  __| |  _  /| |\\/| | | | | . ` | |  | | \n");
  printf("| |  | |/ ____ \\ ____) |  | |  | |____| | \\ \\| |  | |_| |_| |\\  | |__| |   \n");                                                                    
  printf("|_|  |_/_/    \\_\\_____/   |_|  |______|_|  \\_\\_|  |_|_____|_| \\_|_____/   \n");
  printf("\n");  
}


/*-------------------------------------------------------------------------+
| Function: create_sock     após início da execução, criar socket datagrama
+--------------------------------------------------------------------------*/ 
DATAGRAM create_sock(void){
  char mypid[6];                                                              // variável "auxiliar"
  DATAGRAM datsock;                                                           // variável a retornar

  if((datsock.sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ){            // tenta criar um socket datagrama
    perror("[ERRO] Criação de socket datagrama falhou. Tentar Novamente. \n");
    exit(-1);                                                                 // termina o JMMapl
  }
    
  datsock.my_addr_d.sun_family = AF_UNIX;
  memset(datsock.my_addr_d.sun_path, 0, sizeof(datsock.my_addr_d.sun_path));
  strcpy(datsock.my_addr_d.sun_path, CLINAME);
  sprintf(mypid, "%d", getpid());
  strcat(datsock.my_addr_d.sun_path, mypid);                                  // junta o path dos clientes com o pid criando um identificador único
  datsock.addrlen_d = sizeof(datsock.my_addr_d.sun_family) + strlen(datsock.my_addr_d.sun_path);
  
  if(bind(datsock.sd_datagram, (struct sockaddr *)&datsock.my_addr_d, datsock.addrlen_d) < 0 ){
    perror("[ERRO] Bind do socket datagrama. Tentar novamente. \n");
    exit(-1);                                                                 // termina o JMMapl
  }

  printf("[INFO] Socket datagrama criado. Comunicações com o histórico ativadas. \n");

  return datsock;
}

 
/*-------------------------------------------------------------------------+
| Function: monitor        (called from main) 
+--------------------------------------------------------------------------*/ 
void monitor(){
  int argc;
  unsigned long int i;
  static char *argv[ARGVECSIZE + 1], *p;

  printf("%s Type sos for help\n", TitleMsg);
  for(;;){
    printf("\nCmd> ");
    /* Reading and parsing command line  ----------------------------------*/
    if((argc = my_getline(argv, ARGVECSIZE)) > 0){
      for(p = argv[0]; *p != '\0'; *p = tolower(*p), p++);
      for(i = 0; i < NCOMMANDS; i++) 
	if(strcmp(argv[0], commands[i].cmd_name) == 0) 
	  break;
      /* Executing commands -----------------------------------------------*/
      if (i < NCOMMANDS)
	commands[i].cmd_fnct (argc, argv);
      else  
	printf("%s", InvalMsg);
    } /* if my_getline */
  }
}