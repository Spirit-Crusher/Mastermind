#include "client.h"

//char cli_buf[BUFDIM];

//unsigned short int search_com(char *input);

int main(void){

/*



    while(1){
        printf("Introduce command");

        
        
        if(fgets(cli_buf, BUFDIM, stdin) != NULL){
            if(p = strchr(cli_buf, '\n')){  //check existing newline
                *p = 0;
            }else{
                scanf("%*[^\n]");           // reads all up to \n
                scanf("%*c");               // clear upto newline
            }

            switch(search_com(cli_buf)){
                case 1:

                case 2:
            }
            

        }else{
            perror("command");
        }

        
    }
*/
    return 0;
}

/* passar para outro ficheiro */
/*
unsigned short int search_com(char *input){
    int size = sizeof(input);

    if((strstr(input, "cnj") != NULL) && (size <= 9)){
        return 1;
    }else if((strstr(input, "jg") != NULL) && (size <= 8)){
        return 2;
    }else if(strstr(input, "clm")){
        return 3;
    }else if((strstr(input, "mlm") != NULL) && (size <= 7)){
        return 4;
    }else if(strstr(input, "cer")){
        return 5;
    }else if(strstr(input, "aer")){
        return 6;
    }else if(strstr(input, "der")){
        return 7;
    }else if(strstr(input, "tmm")){
        return 8;
    }else if((strstr(input, "ltc") != NULL) && (size <= 5)){
        return 9;
    }else if((strstr(input, "rtc") != NULL) && (size <= 5)){
        return 10;
    }else if(strstr(input, "trh")){
        return 11;
    }else if(strstr(input, "sos")){
        return 12;
    }else if(strstr(input, "help")){
        return 13;
    }else if(strstr(input, "sair")){
        return 14;
    }else{
        return 0;
    }
    
}
*/

/*
    to do next - 2 structs -> cmd stream e datagrama
*/

/* 
    estrutura geral do código para um cliente:
    -> faz as inicializações necessárias quando é criado um novo cliente;
    -> espera instruções por parte do utilizador (constantemente);
    -> 
*/