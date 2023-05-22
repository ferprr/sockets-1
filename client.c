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
    puts(extension);
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
    int selected = 0;

    printf("connected to %s\n", addrstr);

    // Loop principal
    while (1)
    {
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);
        printf("command> ");
        fgets(buf, BUFSZ - 1, stdin);
        buf[strcspn(buf, "\n")] = '\0';

        // Verifica o tipo de comando
        if (strcmp(buf, "exit") == 0)
        {
            printf("Sessão encerrada pelo cliente\n");
            break;
        }
        else if (strncmp(buf, "select file", 11) == 0)
        {
            // Extrai o nome do arquivo
            char *filename = buf + 12;
            if (sscanf(buf, "select file %[^\n]", filename) == 1)
            {
                // Verifica se o arquivo é válido (extensões aceitáveis)
                if (is_valid_extension(filename) == 0)
                {
                    puts(filename);
                    printf("%s not valid!\n", filename);
                    continue;
                }
                else
                {
                    printf("%s selected\n", filename);
                    selected = 1;
                }

                printf("passou nas ver\n");
            }
            else
            {
                printf("no file selected.\n");
            }

            // send(s, buf, strlen(buf), 0);
        }
        else if (strcmp(buf, "send file") == 0 && selected)
        {
            printf("pronto apra enviar\n");
            //     // Recebe o arquivo do cliente
            //     memset(buf, 0, BUFSZ);
            //     if (recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0) < 0)
            //     {
            //         perror("Erro ao receber arquivo do cliente");
            //         break;
            //     }

            //     // Extrai o nome do arquivo
            //     char *filename = buffer + 5;

            //     // Abre o arquivo para escrita
            //     file = fopen(filename, "wb");
            //     if (file == NULL)
            //     {
            //         sprintf(buffer, "error receiving file %s", filename);
            //         send(clientSocket, buffer, strlen(buffer), 0);
            //         break;
            //     }

            //     // Recebe e armazena o conteúdo do arquivo
            //     while (1)
            //     {
            //         memset(buffer, 0, MAX_BUFFER_SIZE);
            //         int bytesRead = recv(clientSocket, buffer, MAX_BUFFER_SIZE, 0);
            //         if (bytesRead < 0)
            //         {
            //             perror("Erro ao receber arquivo do cliente");
            //             break;
            //         }
            //         if (bytesRead == 0)
            //         {
            //             break; // Fim do arquivo
            //         }
            //         fwrite(buffer, 1, bytesRead, file);
            //     }

            //     fclose(file);

            //     // Envia a resposta ao cliente
            //     sprintf(buffer, "file %s received", filename);
            //     send(clientSocket, buffer, strlen(buffer), 0);
            // }
            // else
            // {
            //     sprintf(buffer, "Comando desconhecido");
            //     send(clientSocket, buffer, strlen(buffer), 0);
        }
        else if (strcmp(buf, "send file") == 0 && !selected)
        {
            printf("no file selected.\n");
        }
        else
        {
            printf("invalid command.\n");
        }
    }

    // Encerra a conexão com o cliente
    close(s);
}