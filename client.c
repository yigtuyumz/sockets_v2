#include "include/utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>

void
align_network_byte_order_ip(uint32_t *ipaddr)
{
	*ipaddr = (*ipaddr << 24) | ((*ipaddr >> 8) << 16) | ((*ipaddr >> 16) << 8) | (*ipaddr >> 24);
}

void
align_network_byte_order_port(uint16_t *port)
{
	*port = (*port << 8) | (*port >> 8);
}

int
_is_digit(char c)
{
	return (c <= 57 && c >= 48);
}

uint32_t
ipstr_to_nbo(char *ipaddr)
{
	uint32_t ret_val = 0;
	uint8_t indexes  = 0;
	uint8_t counter     ;
	char octets[4][4];

	while (*ipaddr) {
		if (*ipaddr == '.') {
			indexes += 0x10;
			indexes &= 0xF0;
		} else {
			*(*(octets + (indexes >> 4)) + (indexes & 0x0F)) = *ipaddr;
			indexes++;
		}
		ipaddr++;
	}

	for (counter = 0; counter <= 3; counter++) {
		ret_val |= (uint8_t) utils_atoi(octets[counter]) << (8 * counter);
	}

	return (ret_val);
}

int
main(int argc, char *argv[])
{
	// first arg is the ip addr
	// second arg is the port
        if (argc < 4) {
		errno = 22;
		perror("Not enough arguments to run this program!\n\t \
argv[1] : IP address of the socket\n\t\
argv[2] : Port of the socket\n\t\
argv[3] : Message to be sent\n\n");
		_exit(errno);
        }

	utils_putstr(STDOUT_FILENO, "client\n");



	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		perror("Error while creating socket!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "sock\n");


	// use connect to connect a server from here
	struct sockaddr_in nm;
	struct in_addr nm_ip;
	nm_ip.s_addr = ipstr_to_nbo(argv[1]);

#ifdef __FreeBSD__
	nm.sin_len = sizeof(struct sockaddr_in);
#endif

	nm.sin_family = AF_INET;

	uint16_t porty = (uint16_t) utils_atoi(argv[2]);
	align_network_byte_order_port(&porty);

	nm.sin_port = porty;
	nm.sin_addr = nm_ip;
	utils_strncpy((char * restrict) nm.sin_zero, "wagaCLI\0", 8);

	int conn = connect(sock, (const struct sockaddr *)&nm, (socklen_t) sizeof(struct sockaddr_in));

	if (conn < 0) {
		perror("Connection error!");
		_exit(errno);
	}

	// max_len 50
	// TODO IDEA ????
	//
	// char **utils_strsplit(char *str, unsigned int length);
	//
	// split a string expression by using a length into 2 parts.
	// first string's length should be equal to the 'length' parameter of the function.
	// second string must be NULL if the 'length' > strlen(inputstr). otherwise
	// it must be set to the rest of the input string.
	char *send_buffer = argv[3];



	// send flags
	// #define MSG_OOB         0x00001 Process out-of-band data
	// #define MSG_DONTROUTE   0x00004 Bypass routing, use direct interface
	// #define MSG_EOR         0x00008 Data completes record
	// #define MSG_DONTWAIT    0x00080 Do not block
	// #define MSG_EOF         0x00100 Data completes transaction
	// #define MSG_NOSIGNAL    0x20000 Do not generate SIGPIPE on EOF
	ssize_t send = sendto(sock, (const void *) send_buffer, utils_strlen(send_buffer), MSG_EOR,
			(const struct sockaddr *)&nm, (socklen_t) sizeof(struct sockaddr_in));

	if (send < 0) {
		perror("Send error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "send\n");




	int clo = close(sock);

	if (clo < 0) {
		perror("Close error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "clo\n");


	return (0);
}

