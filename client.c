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
void send_message(int socket, const char *message, const char *filename)
{
    char buffer[BUFSZ];
    sprintf(buffer, "%s %s", filename, message);
    send(socket, buffer, strlen(buffer), 0);
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
    // char *file_path;
    char file_path[100];

    while (1)
    {
        // Solicita ao usuário o comando
        char buf[BUFSZ];
        memset(buf, 0, BUFSZ);

        printf("Digite o comando: ");
        fgets(buf, sizeof(buf), stdin);
        buf[strcspn(buf, "\n")] = '\0'; // Remove a quebra de linha do final

        // Verifica o comando digitado
        const char *command = strtok(buf, " ");
        const char *arg = strtok(NULL, " ");

        if (command == NULL)
        {
            printf("invalid command\n");
            continue;
            // close(s);
            // return 0;
        }

        // Envia o arquivo selecionado para o servidor
        if (strcmp(command, "select") == 0 && strcmp(arg, "file") == 0)
        {
            // Recebe o nome do arquivo selecionado
            // fgets(file_path, sizeof(file_path), stdin);
            // file_path[strcspn(file_path, "\n")] = '\0'; // Remove a quebra de linha do final
            strcpy(file_path, strtok(NULL, "")); // returns pointer to remaining words (filename)
            // puts(file_path);

            // Verifica se o arquivo é válido (extensões aceitáveis)
            if (is_valid_extension(file_path) == 0)
            {
                printf("%s not valid!\n", file_path);
                continue;
            }
            else
            {
                printf("%s selected\n", file_path);
                selected = 1;
                continue;
            }
        }
        else if (strcmp(command, "send") == 0 && strcmp(arg, "file") == 0 && selected)
        {
            // Verifica se o arquivo existe
            FILE *file = fopen(file_path, "rb");
            if (file == NULL)
            {
                printf("%s does not exist\n", file_path);
                continue;
                // close(s);
                // return 0;
            }

            // Envia o comando "send file" para o servidor
            send(s, "send file", BUFSZ, 0);

            // Envia o comando "send file" para o servidor
            send(s, file_path, strlen(file_path), 0);

            // Envia o arquivo para o servidor
            ssize_t bytes_read;
            while ((bytes_read = fread(buf, sizeof(char), BUFSZ, file)) > 0)
            {
                send(s, buf, bytes_read, 0);
            }

            fclose(file);

            // Recebe a resposta do servidor
            ssize_t bytes_received = recv(s, buf, BUFSZ, 0);
            if (bytes_received > 0)
            {
                buf[bytes_received] = '\0';
                printf("Resposta do servidor: %s\n", buf);
            }
        }
        else if (strcmp(command, "send") == 0 && strcmp(arg, "file") == 0 && !selected)
        {
            printf("no file selected.\n");
        }
        // Encerra a conexão com o servidor
        else if (strcmp(command, "exit") == 0)
        {
            send_message(s, "exit", "");
            close(s);
            printf("client closed connection\n");
            return 0;
        }
    }

    return 0;
}