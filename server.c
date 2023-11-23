#include "include/utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>

// 0 enables RELEASE_MODE.
#define ENABLE_DEBUG 0

#if ENABLE_DEBUG == 1
# define DEBUG_ENABLED
# define DEBUG_IP_ADDR "localhost"
# define DEBUG_PORT    "12345"
#endif

#ifdef DEBUG_ENABLED
# undef RELEASE_MODE
# else
# ifndef RELEASE_MODE
/* make with `RELEASE=yes` to ensure code is clean. */
#  define RELEASE_MODE
# endif
#endif

#define RECV_BUFF_LEN 1024


uint8_t
checksum(uint32_t data)
{
	uint8_t sum = 0;
	uint8_t i = sizeof(data);
	while (i--) {
		sum += ((data >> (8 * i)));
	}

	sum--;

	return (0xFFFFFFFF - sum);
}

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

uint32_t
ipstr_to_nbo(char *ipaddr)
{
	if (utils_strcmp(ipaddr, "localhost") == 0) {
		return (0x0100007F);
	}

	uint32_t ret_val = 0;
	uint8_t indexes  = 0;
	uint8_t counter     ;
	char octets[4][4] = { 0 };

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
#ifdef RELEASE_MODE
	if (argc < 3) {
		errno = 22;
		perror("Not enough arguments to run this program!\n\t \
argv[1] : IP address of the socket\n\t \
argv[2] : Port of the socket\n\n");
		_exit(errno);
	}
#endif

	int sock;
	int bnd;
	int listn;
	int accpt;
	ssize_t recieve;
	int clo_accpt;
	int clo_sock;

	if ((sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP)) < 0) {
		perror("Error while creating socket!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "sock\n");

	struct sockaddr_in address;
	uint16_t porty;
	struct sockaddr_in client_address;
	socklen_t client_address_length = sizeof(client_address);
	struct in_addr ip_address;
#ifdef RELEASE_MODE
	ip_address.s_addr = ipstr_to_nbo(argv[1]);
#else
	ip_address.s_addr = ipstr_to_nbo(DEBUG_IP_ADDR);
#endif

#ifdef __FreeBSD__
	address.sin_len = sizeof(struct sockaddr_in);
	client_address.sin_len = sizeof(struct sockaddr_in);
#else
# ifdef __linux__
	size_t sockaddrin_len = sizeof(struct sockaddr_in);
# else
	/* unsupported platform */
# endif
#endif
	address.sin_family = AF_INET;
#ifdef RELEASE_MODE
	porty = (uint16_t) utils_atoi(argv[2]);
#else
	porty = (uint16_t) utils_atoi(DEBUG_PORT);
#endif
	align_network_byte_order_port(&porty);
	address.sin_port = porty;
	address.sin_addr = ip_address;
	utils_strncpy((char * restrict) address.sin_zero, "wagaSERV", 8);


	if ((bnd = bind(sock, (struct sockaddr *)&address, sizeof(address))) < 0) {
		perror("Error while binding!");
		_exit(errno);
	}
	utils_putstr(STDOUT_FILENO, "bnd\n");

	// listens up to 3 clients.
	if ((listn = listen(sock, 3)) < 0) {
		perror("Listen error!");
		_exit(errno);
	}
	utils_putstr(STDOUT_FILENO, "listn\n");


	// accept CREATES A NEW SOCKET!
#ifdef __FreeBSD__
	if (accpt = (accept(sock, (struct sockaddr * restrict)&client_address, (socklen_t *)&client_address.sin_len)) < 0) {
		perror("Accept error!");
		_exit(errno);
	}
#else
# ifdef __linux__
	if ((accpt = accept(sock, (struct sockaddr * restrict)&client_address, (socklen_t *)&client_address_length)) < 0) {
		perror("Accept error!");
		_exit(errno);
	}
# else
	/* unsupported platform */
# endif
#endif
	utils_putstr(STDOUT_FILENO, "accpt\n");

	// data from client is here!!! catch it with recv, recvfrom or recvmsg


	char recv_buffer[RECV_BUFF_LEN];
	// recv flags
	// MSG_OOB                 process out-of-band data
	// MSG_PEEK                peek at incoming message
	// MSG_TRUNC               return real packet or datagram length
	// MSG_WAITALL             wait for full request or error
	// MSG_DONTWAIT            do not block
	// MSG_CMSG_CLOEXEC        set received fds close-on-exec
	// MSG_WAITFORONE          do not block after receiving the first
	//                             message (only for recvmmsg() )
#ifdef __FreeBSD__
	if ((recieve = recvfrom(accpt, (void *)recv_buffer, RECV_BUFF_LEN, 0, (struct sockaddr * restrict)&client_address, (socklen_t * restrict)&address.sin_len)) < 0) {
		perror("Recieve error!");
		_exit(errno);
	}
#else
# ifdef __linux__
	if ((recieve = recvfrom(accpt, (void *)recv_buffer, RECV_BUFF_LEN, 0, (struct sockaddr * restrict)&client_address, (socklen_t * restrict)&sockaddrin_len)) < 0) {
		perror("Recieve error!");
		_exit(errno);
	}
# else
	/* unsupported platform */
# endif
#endif
	utils_putstr(STDOUT_FILENO, "recieve\n");

/* program start */
	do {
		// ! TODO send a message back to the client in here. Use send or sendto.
		utils_vaput(STDOUT_FILENO, "%s%s%s",
			"[MESSAGE FROM THE CLERGY]\n",
			recv_buffer,
			"\n[END OF TRANSMISSION]\n");
	} while (0);
/* program end */


	if ((clo_accpt = close(accpt)) < 0) {
		perror("Close accpt error!");
		_exit(errno);
	}
	utils_putstr(STDOUT_FILENO, "clo_accpt\n");


	if ((clo_sock = close(sock)) < 0) {
		perror("Close sock error!");
		_exit(errno);
	}
	utils_putstr(STDOUT_FILENO, "clo_sock\n");

	return (0);
}

