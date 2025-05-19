/* Note: run this program as root user
 * Author:Subodh Saxena 
 */
#include<stdio.h>
#include<malloc.h>
#include<string.h>
#include<signal.h>
#include<stdbool.h>
#include<sys/socket.h>
#include<sys/types.h>

#include<linux/if_packet.h>
#include<netinet/in.h>		 
#include<netinet/if_ether.h>    // for ethernet header
#include<netinet/ip.h>		// for ip header
#include<netinet/udp.h>		// for udp header
#include<netinet/tcp.h>
#include<arpa/inet.h>           // to avoid warning at inet_ntoa

#include <stdlib.h>
#include "packet.h"

FILE* log_txt, *new_file;
int total;
struct sockaddr saddr;

unsigned char* filebuffer;
unsigned int file_size = 0;
int prev_pack = -1;

void data_process(unsigned char* buffer,int buflen)
{
	struct iphdr *ip = (struct iphdr*)(buffer + sizeof (struct ethhdr));
	++total;
	
	_packet* packet = (_packet*) buffer;
	if (packet->marker == 126) {
		printf("%d %d\n", packet->checksum, prev_pack);
		if (packet->checksum > prev_pack) {
			printf("packet recieved %d\n", packet->size);
			fwrite(packet->data, packet->size, 1, new_file);
			prev_pack = packet->checksum;
		}
		if (packet->type == 15) {
			fclose(new_file);
			exit(-1);
		}
	}
}

int main()
{

	int sock_r,saddr_len,buflen;

	unsigned char* buffer = (unsigned char *)malloc(65536); 
	memset(buffer,0,65536);

	new_file = fopen("newfile.jpg", "w");
	if (!new_file) {
		printf("unable to open newfile.jpg\n");
		return -1;
	}

	log_txt=fopen("log.txt","w");
	if(!log_txt)
	{
		printf("unable to open log.txt\n");
		return -1;

	}

	printf("starting .... \n");

	sock_r=socket(AF_PACKET,SOCK_RAW,htons(ETH_P_ALL)); 
	if(sock_r<0)
	{
		printf("error in socket\n");
		return -1;
	}

	while(1)
	{
		saddr_len=sizeof saddr;
		buflen=recvfrom(sock_r,buffer,65536,0,&saddr,(socklen_t *)&saddr_len);

		if(buflen<0)
		{
			printf("error in reading recvfrom function\n");
			return -1;
		}
		fflush(log_txt);
		data_process(buffer,buflen);
	}

	close(sock_r);// use signals to close socket 
	printf("DONE!!!!\n");
}
