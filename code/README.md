basiei-me neste site para a estrutura:
https://www.lucavall.in/blog/how-to-structure-c-projects-my-experience-best-practices

não esquecer de por as flags de compilação todas

A ideia é ter cada .exe (o server, a aplicação/cliente e o server_log) todos na pasta bin. 
E cada um dos seus ficheiro .c na pasta com o seu nome, a única exceção seriam os header necessários a todos, esses estão na pasta include
Eu gostava de ter um make tal que pudesse fazer: 
make client
make log_server
make servidor
e ele compiláva cada um
e depois uns:
make run_client
make run_log_server
make run_servidor 
para os cimpilar e gcorrer