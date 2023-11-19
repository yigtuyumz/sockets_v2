#include "include/utils.h"
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <errno.h>


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
	if (argc < 3) {
		errno = 22;
		perror("Not enough arguments to run this program!\n\t \
argv[1] : IP address of the socket\n\t \
argv[2] : Port of the socket\n\n");
		_exit(errno);
	}

	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (sock < 0) {
		perror("Error while creating socket!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "sock\n");

	struct sockaddr_in address;
	struct in_addr ip_address;
	ip_address.s_addr = ipstr_to_nbo(argv[1]);
#ifdef __FreeBSD__
	address.sin_len = sizeof(struct sockaddr_in);
#else
# ifdef __linux__
	size_t socklen = sizeof(struct sockaddr_in);
# else
	// unsupported platform
# endif
#endif
	address.sin_family = AF_INET;
	uint16_t porty = (uint16_t) utils_atoi(argv[2]);
	align_network_byte_order_port(&porty);
	address.sin_port = porty;
	address.sin_addr = ip_address;
	utils_strncpy((char * restrict) address.sin_zero, "wagaSERV", 8);

	int bnd = bind(sock, (struct sockaddr *)&address, sizeof(address));

	if (bnd < 0) {
		perror("Error while binding!");
		_exit(errno);

	}

	utils_putstr(STDOUT_FILENO, "bnd\n");

	int listn = listen(sock, 3);

	if (listn < 0) {
		perror("Listen error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "listn\n");


	// accept CREATES A NEW SOCKET!
#ifdef __FreeBSD__
	int accpt = accept(sock, (struct sockaddr * restrict)&address,
			(socklen_t *)&address.sin_len);
#else
# ifdef __linux__
	int accpt = accept(sock, (struct sockaddr * restrict)&address,
			(socklen_t *)&socklen);
# else
	// unsupported platform
# endif
#endif


	if (accpt < 0) {
		perror("Accept error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "accpt\n");

	// data from client is here!!! catch it with recv, recvfrom or recvmsg
	



	char recv_buffer[50];

#ifdef __FreeBSD__
	ssize_t recieve = recvfrom(accpt, (void *)recv_buffer, 50, MSG_WAITALL,
			(struct sockaddr * restrict)&address,
			(socklen_t * restrict)&address.sin_len);
#else
# ifdef __linux__
	ssize_t recieve = recvfrom(accpt, (void *)recv_buffer, 50, MSG_WAITALL,
			(struct sockaddr * restrict)&address,
			(socklen_t * restrict)&socklen);
# else
	// unsupported platform
# endif
#endif
	// recv flags
	// MSG_OOB                 process out-of-band data
	// MSG_PEEK                peek at incoming message
	// MSG_TRUNC               return real packet or datagram length
	// MSG_WAITALL             wait for full request or error
	// MSG_DONTWAIT            do not block
	// MSG_CMSG_CLOEXEC        set received fds close-on-exec
	// MSG_WAITFORONE          do not block after receiving the first
        //                             message (only for recvmmsg() )

	if (recieve < 0) {
		perror("Recieve error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "recieve\n");

// program start////////////////////////////////////////////////////////////////

	utils_putstr(STDOUT_FILENO, recv_buffer);

////////////////////////////////////////////////////////////////////////////////
	int clo_accpt = close(accpt);

	if (clo_accpt < 0) {
		perror("Close accpt error!");
		_exit(errno);
	}
	
	utils_putstr(STDOUT_FILENO, "clo_accpt\n");



	int clo_sock = close(sock);

	if (clo_sock < 0) {
		perror("Close sock error!");
		_exit(errno);
	}

	utils_putstr(STDOUT_FILENO, "clo_sock\n");

	return (0);
}

