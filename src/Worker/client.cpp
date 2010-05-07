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
#include "interface.h"

using namespace std;
int socks;

void * command_receiver_and_responder_stub(void * args)
{
	pthread_t cmd_receiver_thread;
	pthread_create(&cmd_receiver_thread, NULL, &(receive_commands), NULL);
	pthread_join(cmd_receiver_thread, NULL);
}

void send_to_command_view1(string str)
{	
	set_command_view((char *)str.c_str());		
}


void * client_starter(void * args)
{
	int start_send_port = 7000;
	int start_recv_port = 7500;
	
	for(int i = 0; i < 20; i++)
	{ send_ports[i] = start_send_port + i; recv_ports[i] = start_recv_port + i; }
	socks = 10;
	
	cout << "Binding Sockets.." << endl;
	send_to_command_view1("Binding Sockets..");
	for(int i = 0; i < socks; i++)
		bind_sf_socket(i);
	cout << "Sockets Bound.." << endl << endl;	
	send_to_command_view1("Sockets Bound..");
		
	get_supervisor_address();
	cout << "Supervisor address received. " << endl;
	cout << "Ping threads started.." << endl << endl;
	
	send_to_command_view1("Supervisor address received. ");
	send_to_command_view1("Ping threads started..");
	
	pthread_t connection_checker;
	pthread_create(&connection_checker, NULL, &(check_connection_with_supervisor), NULL);
	
	pthread_t ping_supervisor;
	pthread_create(&ping_supervisor, NULL, &(ping_the_supervisor), NULL);
	
	pthread_t crr_stub;
	pthread_create(&crr_stub, NULL, &(command_receiver_and_responder_stub), NULL);
	pthread_join(crr_stub, NULL);

	pthread_join(ping_supervisor, NULL);
	pthread_join(connection_checker, NULL);
	return 0;
}

