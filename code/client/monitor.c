/***************************************************************************
| File: monitor.c
|
| Autor: Carlos Almeida (IST), from work by Jose Rufino (IST/INESC), 
|        from an original by Leendert Van Doorn
| Data:  Nov 2002
***************************************************************************/
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>


/*-------------------------------------------------------------------------+
| Headers of command functions
+--------------------------------------------------------------------------*/ 

       void cmd_sos  (int, char** );
extern void cmd_sair (int, char** );
extern void cmd_test (int, char** );
extern void cmd_cnj  (int, char**);
extern void cmd_jg   (int, char**);
extern void cmd_clm  (int, char**);
extern void cmd_mlm  (int, char**);
extern void cmd_cer  (int, char**);
extern void cmd_aer  (int, char**);
extern void cmd_der  (int, char**);
extern void cmd_tmm  (int, char**);
extern void cmd_ltc  (int, char**);
extern void cmd_rtc  (int, char**);
extern void cmd_trh  (int, char**);


/*-------------------------------------------------------------------------+
| Variable and constants definition
+--------------------------------------------------------------------------*/ 
const char TitleMsg[] = "\n Application Control Monitor\n";
const char InvalMsg[] = "\n Invalid command!";

struct 	command_d {
  void  (*cmd_fnct)(int, char**);
  char*	cmd_name;
  char*	cmd_help;
} 

const commands[] = {
  {cmd_sos,  "sos","                   help"},
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

#define NCOMMANDS  (sizeof(commands)/sizeof(struct command_d))
#define ARGVECSIZE 3
#define MAX_LINE   50


/*-------------------------------------------------------------------------+
| Function: cmd_sos - provides a rudimentary help
+--------------------------------------------------------------------------*/ 
void cmd_sos (int argc, char **argv){
  int i;

  printf("%s\n", TitleMsg);
  for(i = 0; i < NCOMMANDS; i++)
    printf("%s %s\n", commands[i].cmd_name, commands[i].cmd_help);
}


/*-------------------------------------------------------------------------+
| Function: my_getline        (called from monitor) 
+--------------------------------------------------------------------------*/ 
int my_getline (char** argv, int argvsize){
  static char line[MAX_LINE];
  char *p;
  int argc;

  fgets(line, MAX_LINE, stdin);

  /* Break command line into an o.s. like argument vector,
     i.e. compliant with the (int argc, char **argv) specification --------*/

  for (argc = 0,p=line; (*line != '\0') && (argc < argvsize); p = NULL,argc++) {
    p = strtok(p, " \t\n");
    argv[argc] = p;
    if (p == NULL) return argc;
  }
  argv[argc] = p;
  return argc;
}


/*-------------------------------------------------------------------------+
| Function: monitor        (called from main) 
+--------------------------------------------------------------------------*/ 
void monitor(void){
  static char *argv[ARGVECSIZE + 1], *p;
  int argc, i;

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
  } /* forever */
}