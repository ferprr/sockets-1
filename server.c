#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int args, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n");
    exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        usage(argc, argv);
    }

    exit(EXIT_SUCCESS);

    struct sockaddr_storage storage; //estrutura de armazenamento p/ ipv6 ou ipv4
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) //parsing criada em common.c (útil tbm para server) do endereço para dentro da estrutura
    {
        usage(argc, argv);
    }

    // criação do socket
    int s = socket(storage.ss_family, SOCK_STREAM, 0); // CRIA SOCKET CONEXÃO INTERNET COM TCP (lib types e socket)
    if (s == -1)
    {
        logExit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); //instanciação do endereço
    if (0 != bind(s, addr, sizeof(storage))){
        logExit("bind");
    }

    if (0 != listen(s, 10)) {
        logExit("listen");
    }

    char addrstr[BUFSZ];
    addrToStr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n");

    while (1)
    {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)(&storage);

        int client_sock = accept(s, client_addr, sizeof(client_storage)); //quando a conexão é aceita, é criado outro socket para comunicação com o cliente
        if (client_sock == -1)
        {
            logExit("accept");
        }
        
        char client_addrstr[BUFSZ];
        addrToStr(client_addr, client_addrstr, BUFSZ);
        printf("[log] connection fron %s\n", client_addrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        size_t count = recv(client_sock, buf, BUFSZ, 0);
        printf("[msg] %s, %d bytes: %s\n", client_addrstr, (int)count, buf);

        sprintf(buf, "remote endpoint: %s\n", client_addrstr);
        count = send(client_sock, buf, strlen(buf)+1, 0);
        if (count != strlen(buf)+1) {
            logExit("send");
        }
        close(client_sock);
    }
    exit(EXIT_SUCCESS);
}