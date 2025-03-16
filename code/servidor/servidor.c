#include "core_gameplay.h"
#include <stdio.h>

//COLOCAR NO SERVIDOR.C
#define SV_DATAGRAM "/tmp/SV_DATAGRAM"
#define SV_STREAM "/tmp/SV_STREAM"
#define MSG "SERVER IS ALIVE AND WELL"

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
    char buffer_stream[100], buffer_dgram[100]; //posso usar só 1 buffer com mutex i think
    fd_set rfds; //set dos descriptors das sockets
    int client_sockets_fd[NJMAX] = {0}; //array com socket descriptors de todos os clientes
    
    //criação das sockets do servidor
    if((sd_stream = socket(AF_UNIX, SOCK_STREAM, 0)) < 0 ) 
    {
        perror("Erro a criar socket"); 
        exit(-1);
    }
    if((sd_datagram = socket(AF_UNIX, SOCK_DGRAM, 0)) < 0 ) 
    {
        perror("Erro a criar socket"); 
        exit(-1);
    }
  
    //inicialização da estrutura para associar (bind) server à socket
  
    //inicializar socket STREAM
    streamsv_addr.sun_family = AF_UNIX;
    memset(streamsv_addr.sun_path, 0, sizeof(streamsv_addr.sun_path));
    strcpy(streamsv_addr.sun_path, SV_STREAM);
    streamsv_addrlen = sizeof(streamsv_addr.sun_family) + strlen(streamsv_addr.sun_path);
  
    //inicializar socket DATAGRAM
    dgramsv_addr.sun_family = AF_UNIX;
    memset(dgramsv_addr.sun_path, 0, sizeof(dgramsv_addr.sun_path));
    strcpy(dgramsv_addr.sun_path, SV_DATAGRAM);
    dgramsv_addrlen = sizeof(dgramsv_addr.sun_family) + strlen(dgramsv_addr.sun_path);
  
  
    /*streamsv_addr.sun_family = AF_UNIX;
    memset(server_addr.sun_path, 0, sizeof(server_addr.sun_path)); //tenho de inicializar desta maneira? Não posso só fazer strcpy diretamente?
    strcpy(server_addr.sun_path, SERVERNAME);   //definir path para a socket (vai ficar no /tmp)
    server_addrlen = sizeof(server_addr.sun_family) + strlen(server_addr.sun_path);
    */
    unlink(SV_DATAGRAM); //remover ficheiro socket caso este já tenha sido criado previamente
    unlink(SV_STREAM);
  
    //ligar server às socket (bind)
    if(bind(sd_stream, (struct sockaddr *) &streamsv_addr, streamsv_addrlen) < 0 ) 
    {
        perror("Erro no bind"); 
        exit(-1);
    }
    if(bind(sd_datagram, (struct sockaddr *) &dgramsv_addr, dgramsv_addrlen) < 0 ) 
    {
        perror("Erro no bind"); 
        exit(-1);
    }
  
  
    //socket stream vai ficar a ouvir requests para conectar e aceitará apenas um máximo de NJMAX clientes
    if(listen(sd_stream, NJMAX) < 0 ) 
    {
        perror("Erro no listen"); 
        exit(-1);
    }
  
    printf("À espera de receber conexões dos clientes...\n");
  
    //receber mensagens dos clientes
    while(1)
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
        printf("Estou no select\n");
        if((request_made = select(max_sd + 1, &rfds, NULL, NULL, NULL)) < 0 /*&& errno != EINTR*/)
        {
            perror("Erro no select");
        }
  
        //existem pedidos, vamos aceitar conexão
        if(FD_ISSET(sd_stream, &rfds)) 
        {
            client_addrlen = sizeof(client_addr);
            if((new_sock = accept(sd_stream, (struct sockaddr *)&client_addr, &client_addrlen)) < 0) 
            {
                perror("Erro no accept");
                exit(-1);
            }
  
            printf("Novo cliente conectado\n");
  
            // Adicionar o novo cliente ao array de sockets
            for(i = 0; i < NJMAX; ++i) 
            {
                if(client_sockets_fd[i] == 0) //procurar "espaço vazio" no array
                {
                    client_sockets_fd[i] = new_sock;
                    printf("Socket guardada no indice: %d\n", i);
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
                  perror("Cliente desconectou-se");
                  close(s);
                  client_sockets_fd[i] = 0;
              }
              else 
              {
                  printf("SERVER_STREAM: Recebi \"%s\" do cliente com sd \"%d\"\n", buffer_stream, s); //mostrar o que foi recebido
                  
                  //enviar algo para o cliente
                  if(write(s, MSG, strlen(MSG)+1) < 0)    
                  {
                      perror("Erro no write");
                  }
              }
            }
        }
  
        //ler datagramas dos clientes
        if(FD_ISSET(sd_datagram, &rfds))
        {
          client_addrlen = sizeof(client_addr);
          if(recvfrom(sd_datagram, buffer_dgram, sizeof(buffer_dgram), 0, (struct sockaddr *) &client_addr, &client_addrlen) < 0)
          {
            perror("Erro no recvfrom");
          }
          else
          {
            printf("SERVER: Recebi\"%s\" do cliente \"%s\" \n", buffer_dgram, client_addr.sun_path); //mostrar o que foi recebido
                
            //enviar algo para o cliente
            if(sendto(sd_datagram, MSG, strlen(MSG)+1, 0, (struct sockaddr*) &client_addr, client_addrlen) < 0)
            {
                perror("Erro no sendto");
            }
          }
        }
        
        
    }
    
    //NAO ESQUECER DE FECHAR SOCKET E DAR UNLINK
    close(sd_stream);
    close(sd_datagram);
    /*close(s);*/ //aqui já não é ncessário fazer porque fizemos no loop sempre que cliente se disconecta
    unlink(SV_DATAGRAM);
    unlink(SV_STREAM);
}


int main() {

    communications();
    
    return 0;
}