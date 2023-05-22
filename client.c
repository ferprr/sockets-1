#include "common.h"

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define BUFSZ 1024

int is_valid_extension(const char *filename)
{
    const char *valid_extensions[] = {".txt", ".c", ".cpp", ".py", ".tex", ".java"};
    int num_valid_extensions = sizeof(valid_extensions) / sizeof(valid_extensions[0]);

    const char *extension = strrchr(filename, '.'); // retorna ponteiro para '.' encontrado no filename
    if (extension != NULL)
    {
        for (int i = 0; i < num_valid_extensions; i++)
        {
            if (strcmp(extension, valid_extensions[i]) == 0)
            {
                return 1;
            }
        }
    }

    return 0;
}
void usage(int argc, char **argv)
{
    printf("usage: %s <server ip> <server port>\n", argv[0]);
    printf("example: %s 127.0.0.1 51511\n", argv[0]);
    exit(EXIT_FAILURE);
}
int main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage(argc, argv);
    }

    struct sockaddr_storage storage;                // estrutura de armazenamento p/ ipv6 ou ipv4
    if (0 != addrParse(argv[1], argv[2], &storage)) // parsing criada em common.c (útil tbm para server) do endereço para dentro da estrutura
    {
        usage(argc, argv);
    }

    // criação do socket
    int s = socket(storage.ss_family, SOCK_STREAM, 0); // CRIA SOCKET CONEXÃO INTERNET COM TCP (lib types e socket)
    if (s == -1)
    {
        logExit("socket");
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); // instanciação do endereço
    if (0 != connect(s, addr, sizeof(storage)))            // CONECTA NO SOCKET
    {
        logExit("connect");
    }

    char addrstr[BUFSZ];
    addrToStr(addr, addrstr, BUFSZ);

    printf("connected to %s\n", addrstr);

    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("mensagem> ");
    fgets(buf, BUFSZ - 1, stdin);
    size_t count = send(s, buf, strlen(buf) + 1, 0);
    if (count != strlen(buf) + 1)
    {
        logExit("send");
    }

    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    while (1)
    {
        count = recv(s, buf + total, BUFSZ - total, 0);
        if (count == 0)
        {
            // Connection terminated.
            break;
        }
        total += count;
    }
    close(s);

    printf("received %u bytes\n", total);
    puts(buf);
}