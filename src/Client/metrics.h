/* For computing the Network / Performance
 * Metric. Helps in Load Balancing */

#include <cmath>
#include <cstdlib>

#include "config.h"

char buf[MAXBUFLEN];
double nw_metric, pf_metric;

struct nw_metric_params
{
        int packet_size, packet_count; 
        char *server_add;
};

double performance_metric()
{

	int start = clock(), i, j, k, l;
	double p=1.231;
	
	int mat1[3][3], mat2[3][3];
	mat1[0][0] = 1; mat1[0][1] = 4; mat1[0][2] = 9;
	mat1[1][0] = 2; mat1[1][1] = 5; mat1[1][2] = 8;
	mat1[2][0] = 3; mat1[2][1] = 6; mat1[2][2] = 7;
	
	mat2[0][0] = 9; mat2[0][1] = 5; mat2[0][2] = 4;
	mat2[1][0] = 2; mat2[1][1] = 1; mat2[1][2] = 5;
	mat2[2][0] = 4; mat2[2][1] = 4; mat2[2][2] = 6;
	
	for(i = 1; i < PERFMETRICITERATIONS; i++)
	{
		p = p * p * pow(p,i);
		p = p / (p * 0.912);
		
		int mat[3][3];
		
		for(j = 0; j < 3; j++)
			for(k = 0; k < 3; k++)
			{
				int temp = 0;
				for(l = 0; l < 3; l++)
					temp += mat1[j][l] * mat2[l][k];
				
				mat[j][k] = temp;
			}
		
		for(j = 0; j < 3; j++)
			for(k = 0; k < 3; k++)
				mat1[j][k] = (mat[j][k] % 100007);
		
	}
	int end = clock();
	
	double t = (end - start) * 1.0 / CLOCKS_PER_SEC;
	t = 1 / t;
	return t;
}

double network_metric(int packet_size, int packet_count, char *server_add )
{
	int temp_garb;
	char reliability_buf[100], latency_buf[100], tempbuf[100];
	char command[100];
	int packets_lost, reliability;
	double min_latency, avg_latency, max_latency, mdev_latency;
	double network_metric_value = 0;
	FILE *pingreader;
	
	sprintf(command, "ping -q -s %d -c %d %s > pingstats.tmp", 
			packet_size, packet_count, server_add);
	
	system(command);
	pingreader = fopen("pingstats.tmp", "r");
	fseek(pingreader, 0, SEEK_SET);
	
	/*
		Strictly depends upon the following format:
		PING 127.0.0.1 (127.0.0.1) 20(48) bytes of data.

		--- 127.0.0.1 ping statistics ---
		1 packets transmitted, 1 received, 0% packet loss, time 0ms
		rtt min/avg/max/mdev = 0.030/0.030/0.030/0.000 ms
	 */
	
	fgets(tempbuf, 100, pingreader);
	fgets(tempbuf, 100, pingreader);
	fgets(tempbuf, 100, pingreader);
	fgets(reliability_buf, 100, pingreader);
	fgets(latency_buf, 100, pingreader);
	
	sscanf(reliability_buf, "%d packets transmitted, %d received, \
			%d%% packet loss, time %dms", &temp_garb, &temp_garb, 
			&packets_lost, &temp_garb);
	sscanf(latency_buf, "rtt min/avg/max/mdev = %lf/%lf/%lf/%lf ms", 
			&min_latency, &avg_latency, &max_latency, &mdev_latency);
	
	/*
			Network Metric Computation
			n = reliability * 1 / ( avg_latency + 2 * mdev_latency)
	*/
	
	reliability = 100 - packets_lost;

	network_metric_value = reliability * 1.0 /((avg_latency + 2 * mdev_latency) * 1.0);
	return network_metric_value;
}


void * recalculate_metrics(void *r)
{
        struct nw_metric_params *n = (struct nw_metric_params *)(r);
        
        while(1)
        {
                
                usleep(RECALCULATIONTIMEGAP);
                pf_metric = performance_metric();
                nw_metric = network_metric(n->packet_size, n->packet_count, n->server_add);	
                printf("Metrics recalculated. Performance Metric: %lf  Network Metric: %lf\n", pf_metric, nw_metric);
	}
}
