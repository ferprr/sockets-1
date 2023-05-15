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
void main(int argc, char **argw)
{
    if (argc < 3)
    {
        usage();
    }

    // criação do socket
    int s = socket(AF_INET, SOCK_STREAM, 0); // CRIA SOCKET CONEXÃO INTERNET COM TCP (lib types e socket)
    if (s == -1)
    {
        logExit("socket");
    }

    if (0 != connect(0, addr, sizeof(addr))) // CONECTA NO SOCKET
    {
        logExit("connect");
    }

    char addrstr[BUFSZ];
    addrToStr(addr, addrstr, BUFSZ);

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