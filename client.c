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
    int selected = 0;

    printf("connected to %s\n", addrstr);

    // Loop principal
    while (1)
    {
        char buf[BUFSZ];
        char selected_file[100];

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
                    printf("%s not valid!\n", filename);
                    continue;
                }
                else
                {
                    printf("%s selected\n", filename);
                    selected = 1;
                    strcpy(selected_file, filename); //na proxima iteração não vai estar com essa cópia, limpou
                }
            }
            else
            {
                printf("no file selected.\n");
            }
        }
        else if (strcmp(buf, "send file") == 0 && selected)
        {
            // Envia o nome do arquivo para o servidor
            if (send(s, selected_file, strlen(selected_file), 0) < 0)
            {
                perror("Erro ao enviar nome do arquivo para o servidor");
                break;
            }

            // Abre o arquivo para leitura
            FILE *file = fopen(selected_file, "rb");
            if (file == NULL)
            {
                //puts(selected_file);
                printf("%s does not exist\n", selected_file);
                continue;
            }

            // Lê e envia o conteúdo do arquivo para o servidor
            while (1)
            {
                // Lê dados do arquivo
                int bytesRead = fread(buf, 1, BUFSZ, file);

                // Envia dados para o servidor
                if (send(s, buf, bytesRead, 0) < 0)
                {
                    perror("Erro ao enviar arquivo para o servidor");
                    break;
                }

                // Verifica se chegou ao final do arquivo
                if (bytesRead < BUFSZ)
                {
                    break;
                }
            }

            fclose(file);
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