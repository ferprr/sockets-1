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
    char *file_path;

    while (1)
    {
        char buf[BUFSZ];
        char command[15];

        memset(buf, 0, BUFSZ);

        // Recebe o comando do cliente
        ssize_t bytes_received = recv(csock, buf, BUFSZ, 0);
        if (bytes_received > 0)
        {
            buf[bytes_received] = '\0';

            strncpy(command, buf, 9);

            // Verifica se o comando é "send file"
            if (strcmp(command, "send file") == 0)
            {

                // Extrai o nome do arquivo
                file_path = buf + 10;

                printf("file %s received\n", file_path);

                // Verifica se o arquivo já existe no servidor
                FILE *file = fopen(file_path, "rb");
                if (file)
                {
                    printf("file %s overwritten\n", file_path);
                }

                // Cria um novo arquivo no servidor
                file = fopen(file_path, "wb");
                if (file == NULL)
                {
                    printf("error receiving file");
                }
                else
                {
                    // Recebe o conteúdo do arquivo e escreve no disco
                    memset(buf, 0, BUFSZ);
                    while ((bytes_received = recv(csock, buf, BUFSZ, 0)) > 0)
                    {
                        fwrite(buf, sizeof(char), bytes_received, file);
                    }
                }
                fclose(file);
            }

            strcpy(command, strtok(buf, " "));
            strncpy(command, buf, 4);

            // Verifica se o comando é "exit"
            if (strcmp(command, "exit") == 0)
            {
                // Encerra a conexão com o cliente
                printf("client closed connection\n");
                close(csock);
                close(s);
                return 0;
            }
        }
    }

    return 0;
}