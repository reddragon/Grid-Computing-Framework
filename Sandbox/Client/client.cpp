#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <math.h>
#include <iostream>
#include <sstream>
#include <vector>
#include <string>
#include <sstream>

#include "task_ce.h"
#include "metrics.h"
#include "file_transfer.h"
#include "config.h"
#include "ping_and_command.h"

using namespace std;

void * command_receiver_and_responder_stub(void * args)
{
	pthread_t cmd_receiver_thread;
	pthread_create(&cmd_receiver_thread, NULL, &(receive_commands), NULL);
	pthread_join(cmd_receiver_thread, NULL);
}

int main()
{
	bind_sf_socket();
	get_server_address();
	pthread_t connection_checker;
	pthread_create(&connection_checker, NULL, &(check_connection_with_server), NULL);
	
	pthread_t ping_server;
	pthread_create(&ping_server, NULL, &(ping_the_server), NULL);
	
	pthread_t crr_stub;
	pthread_create(&crr_stub, NULL, &(command_receiver_and_responder_stub), NULL);
	pthread_join(crr_stub, NULL);

	pthread_join(ping_server, NULL);
	pthread_join(connection_checker, NULL);
	return 0;
}

