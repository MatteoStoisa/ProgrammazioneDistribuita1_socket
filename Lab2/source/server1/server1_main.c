#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>
#include <sys/time.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <unistd.h>

#include "../send_file.h"
#include "../error_manage.h"
#include "../sockwrap.h"

char *prog_name;

int long_output = 1;

int main(int argc, char *argv[]) //in *argv: nomeProgramma porta
{
	socklen_t address_length;
	char *buf;
	int server_socket_figlio;

	/*struct timeval tval;
	fd_set cset;*/

	printf("* SERVER TCP *\n");

	//controllo parametri linea di comando
	if (argc != 2)
		serverError(1);
	if (atoi(argv[1]) < 1024)
		serverError(5);
	if (long_output)
		printf("PASS parametri linea di comando\n");

	//creazione socket
	int server_socket_passivo = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (server_socket_passivo < 0)
		serverError(2);
	if (long_output)
		printf("PASS creazione socket\n");

	//definizione indirizzo
	struct sockaddr_in server_address; //definizione indirizzo
	server_address.sin_family = AF_INET;
	server_address.sin_port = htons(atoi(argv[1]));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	if (long_output)
		printf("PASS definizione indirizzo\n");

	//binding socket
	if (bind(server_socket_passivo, (struct sockaddr *)&server_address, sizeof(server_address)) < 0)
		serverError(3);
	if (long_output)
		printf("PASS binding socket\n");

	//inizio ascolto
	if (listen(server_socket_passivo, 100) < 0)
		serverError(4);
	if (long_output)
		printf("PASS inizio ascolto\n");

	/*t = 15;
	tval.tv_sec = t; 
	tval.tv_usec = 0;

	FD_ZERO(&cset); 
	FD_SET(so, &cset);*/

	while (1)
	{
		printf("- IN ATTESA DI CONNESSIONE -\n");

		//accettazione connesione
		address_length = sizeof(struct sockaddr_in);
		server_socket_figlio = accept(server_socket_passivo, (struct sockaddr *)&server_address, &address_length);
		if (server_socket_figlio < 0)
			serverError(5);
		printf("- CONNESSIONE STABILITA -\n");

		while (1)
		{
			//ricezione G E T ' '
			buf = malloc(4 * sizeof(char));
			if (read(server_socket_figlio, buf, 4) <= 0)
			{
				free(buf);
				close(server_socket_figlio);
				printf("- CONNESSIONE CHIUSA -\n");
				break;
			}
			if (strcmp(buf, "GET ") != 0)
			{
				serverSendErr(server_socket_figlio);
				serverError(6);
			}
			free(buf);
			if (long_output)
				printf("PASS GET' ' ricevuto\n");
			server_send_file_to_client(server_socket_figlio);
		}
	}

	return 0;
}
