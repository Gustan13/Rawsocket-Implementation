/* Note: run this program as root user
 * Author:Subodh Saxena 
 */
#include <stdio.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>

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

struct ifreq ifreq_i; 
int sock_raw;
unsigned char *sendbuff;

int total_len=0,send_len;

int create_raw_socket(char *interface_name) {
	int s = socket(AF_PACKET, SOCK_RAW, IPPROTO_RAW);
	if (s == -1) {
		fprintf(stderr, "Could not create rawsocket.\n");
		exit(-1);
	}
	
	int ifindex = if_nametoindex(interface_name);

	struct sockaddr_ll address = {0};
	address.sll_family = AF_PACKET;
	address.sll_protocol = htons(ETH_P_ALL);
	address.sll_ifindex = ifindex;

	if (bind(s, (struct sockaddr*) &address, sizeof(address)) == -1) {
		fprintf(stderr, "Could not bind socket\n");
		exit(-1);
	}

	struct packet_mreq mr = {0};
	mr.mr_ifindex = ifindex;
	mr.mr_type = PACKET_MR_PROMISC;
	if (setsockopt(s, SOL_PACKET, PACKET_ADD_MEMBERSHIP, &mr, sizeof(mr)) == -1) {
		fprintf(stderr, "Could not finish setsockopt: was the network interface correctly specified?\n");
		exit(-1);
	}

	return s;
}

void get_eth_index(struct ifreq ifreq_i, int sock_raw)
{
	memset(&ifreq_i, 0, sizeof(ifreq_i));
	strncpy(ifreq_i.ifr_name, "eth0", IFNAMSIZ - 1);

	if((ioctl(sock_raw, SIOCGIFINDEX, &ifreq_i)) < 0) {
		fprintf(stderr, "Error in index ioctl reading$");
	}
	return ifreq_i;
}

int main()
{
	FILE* pixelart = fopen("landscape.jpeg", "r");
	if (pixelart == NULL) {
		return -1;
	}
	
	sock_raw = create_raw_socket();
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
