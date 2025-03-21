#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/un.h>
#include "socket_server.h"
#include "util.h"
#include "logs.h"

int sock_fd, sock_simulador_fd;

void openSocket()
{
    int clilen, childpid, servlen;
    struct sockaddr_un cli_addr, serv_addr;

    if ((sock_fd = socket(AF_UNIX, SOCK_STREAM, 0)) < 0)
    {
        printf("Erro ao abrir socket.\n");
        exit(1);
        return;
    }

    printf("Socket aberta.\n");

    bzero((char *)&serv_addr, sizeof(serv_addr)); // limpeza preventiva
    serv_addr.sun_family = AF_UNIX;
    strcpy(serv_addr.sun_path, UNIXSTR_PATH); // ficheiro para clientes identificarem servidor

    servlen = strlen(serv_addr.sun_path) + sizeof(serv_addr.sun_family);
    unlink(UNIXSTR_PATH);

    if (bind(sock_fd, (struct sockaddr *)&serv_addr, servlen) < 0)
    {
        printf("Erro ao fazer vincular endereço local.\n");
        exit(1);
        return;
    }

    printf("Endereço local vinculado.\n");

    listen(sock_fd, 1); // aceita 1 cliente

    clilen = sizeof(cli_addr);

    if (fork() == 0)
    {
        if (execl("./simulador", "simulador", NULL) < 0)
        {
            printf("Erro ao executar o simulador.\n");
            exit(1);
            return;
        }

        exit(0);
        return;
    }

    printf("Simulador aberto, aguardando conecxão do simulador...\n");

    sock_simulador_fd = accept(sock_fd, (struct sockaddr *)&cli_addr, &clilen);

    if (sock_simulador_fd < 0)
    {
        printf("Erro ao aceitar conecxão do simulador.\n");
        exit(1);
        return;
    }

    printf("Simulador conectado.\n");
    addLogMessage("Simulação iniciada.\n");

    while (1)
    {
        char buf[MAX_LEN];
        int n = readline(sock_simulador_fd, buf, MAX_LEN);

        if (n < 0)
            printf("str_cli: readline error: %i\n", n);

        buf[n - 1] = '\n';
        buf[n] = 0;

        if (strcmp(buf, "exit\n") == 0)
            break;

        fputs(buf, stdout);
        addLogMessage(buf);
    }

    addLogMessage("Fim da simulação.\n\n\n\n");
    printf("Simulador desconectado.\n");
}

void closeSocket()
{
    printf("Fechando socket...\n");
    close(sock_fd);
    close(sock_simulador_fd);
    printf("Socket fechada.\n");
}
