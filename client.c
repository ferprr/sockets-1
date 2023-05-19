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

    // char addrstr[BUFSZ];
    // addrToStr(addr, addrstr, BUFSZ); //criada em common.c (útil tbm para server)

    // printf("connected to %s\n", addrstr);

    // int count = send(s, buf, strlen(buf) + 1, 0); // retorna número de bytes transmitidos na rede (tem q ser strlen + 1, q foi o q mandamos, se não deu erro)
    // if (count != strlen(buf) + 1)
    // {
    //     logExit("send");
    // }
    // memset(buf, 0, BUFSZ);
    // unsigned total = 0;
    // while (1)
    // {
    //     count = recv(s, buf + total, BUFSZ - total, 0); // recebe resposta servidor
    //     if (count == 0)                                 // não recebeu nada, então conexão fechou
    //     {
    //         // connection terminated
    //         break;
    //     }
    //     total += count;
    // }
    // close(s); // fecha conexão com socket

    // printf("received %u bytes\n", total);
    // puts(buf);

    // lendo comando do teclado
    char buf[BUFSZ];
    memset(buf, 0, BUFSZ);
    printf("message> ");
    fgets(buf, BUFSZ - 1, stdin);

    const char *command = strtok(buf, " "); // separa string em pedaços, nos espaços, por ex "select"
    const char *param = strtok(NULL, " ");  // separa string em pedaços, de onde parou na última vez (acima), por ex "file"

    if (command == NULL)
    {
        logExit("invalid command."); // só teste, remover dps
        close(s);
    }

    if (strcmp(command, "select") == 0)
    {
        if (param == NULL || strcmp(param, "file") != 0)
        {
            logExit("no file selected!"); // só teste remover dps
            close(s);
        }

        // caminho (COMPLETO) do arquivo
        char file_path[256];
        printf("file path: ");
        fgets(file_path, sizeof(file_path), stdin);

        // verifica se arquivo existe
        FILE *file = fopen(file_path, "rb");
        if (file == NULL)
        {
            printf("[%s] do not exist\n", file_path);
            logExit("fopen");
        }

        // verifica extensão do arquivo
        if (!is_valid_extension(file_path))
        {
            printf("[%s] not valid!", file_path);
            fclose(file);
        }

        printf("[%s] selected", file_path); // ready to send file

        // envia comando "send file" para servidor
        send(s, command, strlen(command), 0);

        // envia nome do arquivo para servidor
        send(s, file_path, strlen(file_path), 0);

        // envia conteúdo do arquivo
        ssize_t bytes_read;
        while ((bytes_read = fread(buf, sizeof(char), BUFSZ, file)) > 0)
        {
            send(s, buf, bytes_read, 0);
        }

        fclose(file);
    }
    else
    {
        logExit("file");
    }

    // receiving answer from the server

    exit(EXIT_SUCCESS);
}