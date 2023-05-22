#include "common.h"

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int argc, char **argv)
{
    printf("usage: %s <v4|v6> <server port>\n", argv[0]);
    printf("example: %s v4 51511\n", argv[0]);
    exit(EXIT_FAILURE);
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
        struct sockaddr_storage cstorage;
        struct sockaddr *caddr = (struct sockaddr *)(&cstorage);
        socklen_t caddrlen = sizeof(cstorage);

        int csock = accept(s, caddr, &caddrlen);
        if (csock == -1)
        {
            logExit("accept");
        }

        char caddrstr[BUFSZ];
        addrToStr(caddr, caddrstr, BUFSZ);
        printf("[log] connection from %s\n", caddrstr);

        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        // Recebe dados do cliente
        int bytesRead = recv(csock, buf, BUFSZ, 0);
        if (bytesRead < 0)
        {
            perror("Erro ao receber dados do cliente");
            break;
        }
        else if (bytesRead == 0)
        {
            // Conexão encerrada pelo cliente
            break;
        }

        // Abre o arquivo para escrita
        FILE *file = fopen("x.txt", "wb");
        if (file == NULL)
        {
            printf("Erro ao abrir arquivo\n");
            continue;
        }

        // Escreve os dados no arquivo
        fwrite(buf, 1, bytesRead, file);

        // Verifica se chegou ao final do arquivo
        if (bytesRead < BUFSZ)
        {
            break;
        }

        fclose(file);
    }

    // // Envia a resposta ao cliente
    // sprintf(buf, "Arquivo %s recebido com sucesso", filename);
    // send(csock, buf, strlen(buf), 0);
}