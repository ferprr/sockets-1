#include "common.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <pthread.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}

struct client_data
{
    int client_sock;
    struct sockaddr_storage storage;
};

void * client_thread(void *data)
{
    struct client_data *cdata = (struct client_data *) data;
    struct sockaddr *client_addr = (struct sockaddr *)(&cdata->storage);

    char client_addrstr[BUFSZ];
    addrToStr(client_addr, client_addrstr, BUFSZ);
    printf("[log] connection from %s\n", client_addrstr);

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    size_t count = recv(cdata->client_sock, buf, BUFSZ - 1, 0);
    printf("[msg] %s, %d bytes: %s\n", client_addrstr, (int)count, buf);

    sprintf(buf, "remote endpoint: %.1000s\n", client_addrstr);
    count = send(cdata->client_sock, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1)
    {
        logExit("send");
    }
    close(cdata->client_sock);

    pthread_exit(EXIT_SUCCESS);
}

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;                           // estrutura de armazenamento p/ ipv6 ou ipv4
    if (0 != server_sockaddr_init(argv[1], argv[2], &storage)) // parsing criada em common.c (útil tbm para server) do endereço para dentro da estrutura
    {
        usage(argc, argv);
    }

    // criação do socket
    int s = socket(storage.ss_family, SOCK_STREAM, 0); // CRIA SOCKET CONEXÃO INTERNET COM TCP (lib types e socket)
    if (s == -1)
    {
        logExit("socket");
    }

    int enable = 1;
    if (0 != setsockopt(s, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int))) // reutilizar porto
    {
        logExit("setsocketopt");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); // instanciação do endereço
    if (0 != bind(s, addr, sizeof(storage)))
    {
        logExit("bind");
    }

    if (0 != listen(s, 10))
    {
        logExit("listen");
    }

    char addrstr[BUFSZ];
    addrToStr(addr, addrstr, BUFSZ);
    printf("bound to %s, waiting connections\n", addrstr);

    while (1)
    {
        struct sockaddr_storage client_storage;
        struct sockaddr *client_addr = (struct sockaddr *)(&client_storage);
        socklen_t client_storage_len = sizeof(client_storage);

        int client_sock = accept(s, client_addr, &client_storage_len); // quando a conexão é aceita, é criado outro socket para comunicação com o cliente
        if (client_sock == -1)
        {
            logExit("accept");
        }

        // servidor está pronto para receber conexões de clientes

        struct client_data *cdata = malloc(sizeof(*cdata));
        if (!cdata)
        {
            logExit("malloc");
        }
        cdata->client_sock = client_sock;
        memcpy(&(cdata->storage), &client_storage, sizeof(client_storage));

        pthread_t tid;
        pthread_create(&tid, NULL, client_thread, cdata); // cria outro fluxo de execução para cada thread, mas o fluxo principal continua executando while e recebendo novas conexões
    }
    exit(EXIT_SUCCESS);
}