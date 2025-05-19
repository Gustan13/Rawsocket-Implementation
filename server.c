/* Note: run this program as root user
 * Author:Subodh Saxena 
 */
#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <errno.h>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/ioctl.h>

#include <net/if.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>

#include <linux/if_packet.h>

#include <arpa/inet.h>

#include "packet.h"

struct ifreq ifreq_c,ifreq_i,ifreq_ip; /// for each ioctl keep diffrent ifreq structure otherwise error may come in sending(sendto )
int sock_raw;
unsigned char *sendbuff;

int total_len=0,send_len;

void get_eth_index()
{
	memset(&ifreq_i,0,sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name,"eth0",IFNAMSIZ-1);

	if((ioctl(sock_raw,SIOCGIFINDEX,&ifreq_i))<0)
		printf("error in index ioctl reading");

	printf("index=%d\n",ifreq_i.ifr_ifindex);
}

unsigned short checksum(unsigned short* buff, int _16bitword)
{
	unsigned long sum;
	for(sum=0;_16bitword>0;_16bitword--)
		sum+=htons(*(buff)++);
	do
	{
		sum = ((sum >> 16) + (sum & 0xFFFF));
	}
	while(sum & 0xFFFF0000);

	return (~sum);


	
}

int main()
{
	FILE* pixelart = fopen("landscape.jpeg", "r");
	if (pixelart == NULL) {
		return -1;
	}
	
	sock_raw=socket(AF_PACKET,SOCK_RAW,IPPROTO_RAW);
	if(sock_raw == -1)
		printf("error in socket");

	sendbuff=(unsigned char*)malloc(134); // increase in case of large data.Here data is --> AA  BB  CC  DD  EE
	memset(sendbuff,0,134);


	get_eth_index();  // interface number

	struct sockaddr_ll sadr_ll;
	sadr_ll.sll_ifindex = ifreq_i.ifr_ifindex;
	sadr_ll.sll_halen   = ETH_ALEN;

	printf("sending...\n");
	char* buffer = malloc(138);
	memset(buffer, 0, 138);
	_packet* packet = (_packet*)buffer;
	int read_size = fread(packet->data, 127, 1, pixelart);
	int cur = 0;
	
	int packet_num = 0;
	
	do {
		packet->marker = (char)126;
		packet->size = read_size*127;
		packet->checksum = packet_num;
		printf("%d - %d\n", packet->size, packet_num);
		packet->type = 2;
		send_len = sendto(sock_raw,buffer,138,0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
			if(send_len<0)
			{
				printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
				return -1;

			}
		memset(buffer, 0, 134);
		cur = ftell(pixelart);
		read_size = fread(packet->data, 127, 1, pixelart);
		packet_num++;
	} while (read_size != 0);
	
	int end = ftell(pixelart);
	int total = end - cur;
	fseek(pixelart, 0, cur);
	
	packet->marker = (char)126;
	packet->size = total;
	packet->type = (char)15;
	packet->checksum = packet_num;
	
	printf("remaining: %d %d\n", total, packet_num);

	send_len = sendto(sock_raw,buffer,138,0,(const struct sockaddr*)&sadr_ll,sizeof(struct sockaddr_ll));
	if(send_len<0)
	{
		printf("error in sending....sendlen=%d....errno=%d\n",send_len,errno);
		return -1;
	}
}
