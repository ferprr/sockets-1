#include <stdlib.h>
#include <stdio.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>

#define BUFSZ 1024

void usage(int args, char **argw)
{
    printf("usage: %s <server ip> <server port>");
    printf("example: %s 127.0.0.1 51511");
    exit(EXIT_FAILURE);
}
void logExit(const char *msg)
{
    perror(msg);
    exit(EXIT_FAILURE);
}
void main(int argc, char **argv)
{
    if (argc < 3)
    {
        usage(argc, argv);
    }

    // criação do socket
    int s = socket(AF_INET, SOCK_STREAM, 0); // CRIA SOCKET CONEXÃO INTERNET COM TCP (lib types e socket)
    if (s == -1)
    {
        logExit("socket");
    }

    struct sockaddr_storage storage; //estrutura de armazenamento p/ ipv6 ou ipv4
    if (0 != addrParse(argv[1], argv[2], &storage)) //parsing //criada em common.c (útil tbm para server) do endereço para dentro da estrutura
    {
        usage(argc, argv);
    }

    struct sockaddr *addr = (struct sockaddr *)(&storage); //instanciação do endereço
    if (0 != connect(0, addr, sizeof(storage))) // CONECTA NO SOCKET
    {
        logExit("connect");
    }

    char addrstr[BUFSZ];
    addrToStr(addr, addrstr, BUFSZ); //criada em common.c (útil tbm para server)

    printf("connected to %s\n");

    // client manda o dado
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("message> ");
    fgets(buf, BUFSZ - 1, stdin);                 // lendo do teclado
    int count = send(s, buf, strlen(buf) + 1, 0); // retorna número de bytes transmitidos na rede (tem q ser strlen + 1, q foi o q mandamos, se não deu erro)
    if (count != strlen(buf) + 1)
    {
        logExit("send");
    }
    memset(buf, 0, BUFSZ);
    unsigned total = 0;
    while (1)
    {
        count = recv(s, buf + total, BUFSZ - total, 0); // recebe resposta servidor
        if (count == 0)                                 // não recebeu nada, então conexão fechou
        {
            // connection terminated
            break;
        }
        total += count;
    }
    close(s); // fecha conexão com socket

    printf("received %u bytes\n", total);
    puts(buf);

    exit(EXIT_SUCCESS);
}