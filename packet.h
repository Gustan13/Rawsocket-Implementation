typedef struct {
	unsigned char marker;
	unsigned int size:7;
	unsigned char type:4;
	int checksum;
	unsigned char data[128];
} _packet;
