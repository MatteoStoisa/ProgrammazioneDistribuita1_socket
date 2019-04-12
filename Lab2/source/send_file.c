#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <arpa/inet.h>
#include <string.h>
#include <time.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/stat.h>

#include "receive_file.h"
#include "error_manage.h"
#include "sockwrap.h"
#include "errlib.h"

int extern long_output;

int server_send_file_to_client(int socket)
{
    int status_bar1, status_bar2;
    unsigned long int i, uliCount;
    char nome_file[50], *buf;
    FILE *file;
    uint32_t size,timestamp;
    struct stat st;

    i = 0;
    while (1)
    {
        //ricezione nome file
        read(socket, &nome_file[i], 1);
        if (nome_file[i] == 13)
        {
            nome_file[i] = '\0';
            if (long_output)
                printf("PASS nome file ricevuto\n");
            //ricezione (CR) LF
            read(socket, &nome_file[i + 1], 1);
            if (nome_file[i + 1] == 10)
            {
                if (long_output)
                    printf("PASS CR LF ricevuti\n");
                break;
            }
            else
            {
                serverSendErr(socket);
                serverError(6);
            }
        }
        else
        {
            i++;
            if (i == 50)
            {
                serverSendErr(socket);
                serverError(8);
            }
        }
    }

    printf("- RICHIESTO FILE '%s' -\n", nome_file);
    //apertura file
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        serverError(7);
    }
    if (long_output)
        printf("PASS file aperto\n");
    //invio + O K
    if (write(socket, "+OK\r\n", 5) != 5)
        return (-1);
    if (long_output)
        printf("PASS +OK CR LF inviato\n");
    //conteggio dimensione
    uliCount = 0;
    buf = malloc(sizeof(char));
    while (fscanf(file, "%c",buf) != EOF)
    {
        uliCount++;
    }
    fclose(file);
    free(buf);
    size = htonl(uliCount);
    write(socket, &size, 4);
    if (long_output)
        printf("PASS dimensione '%lu' Byte inviata\n", uliCount);
    printf("- INVIO IN CORSO ");
    file = fopen(nome_file, "r");
    if (file == NULL)
    {
        serverSendErr(socket);
        serverError(7);
    }
    //scansione-invio file
    status_bar1 = uliCount / 10;
    status_bar2 = 0;
    buf = malloc(sizeof(char));
    for (i = 0; i < uliCount; i++)
    {
        fflush(stdout);
        fscanf(file, "%c", buf);
        write(socket, buf, 1);
        //barra di aggiornamento
        if (i == status_bar2)
        {
            printf("#");
            status_bar2 += status_bar1;
        }
    }
    free(buf);
    fclose(file);
    printf(" -\n");
    if (long_output)
        printf("PASS file inviato\n");
    //invio timestamp

    int ultima_modifica = st.st_mtime;
    timestamp = htonl(ultima_modifica);
    //???
    //time_t ts = timestamp;
    //struct tm *timestamp_format;
    write(socket, &timestamp, 4);
    //timestamp_format = localtime(&ts);
    //strftime(timestamp, sizeof(timestamp), "%d.%m.%Y %H:%M:%S", timestamp_format);
    //printf("- ULTIMA MODIFICA FILE: %s\n", timestamp);
}