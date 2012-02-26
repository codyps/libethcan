#include <pthread.h>
#include <stdlib.h>
#include <sys/uio.h> /* iovec */
#include <arpa/inet.h> /* htonl */
#include <stdio.h> /* sprintf */
#include "ethcan.h"
#include "ellsi.h"


#define ECAN_QUEUE_IN_SIZE  4096
#define ECAN_QUEUE_OUT_SIZE 4096

typedef struct _ecan_in {
	char   queue[ECAN_QUEUE_IN_SIZE];
	size_t queue_len;
} _ecan_in;

typedef struct _ecan_out {
	char   queue[ECAN_QUEUE_OUT_SIZE];
	size_t queue_len;
} _ecan_out;

static int _ecan_out_init(_ecan_out *out)
{
	return 0;
}

static void _ecan_out_destroy(_ecan_out *out)
{
	return;
}

static int _ecan_in_init(_ecan_in *in)
{
	return 0;
}

static void _ecan_in_destroy(_ecan_in *in)
{
	return;
}

struct ecan_connection_t {
	int fd;

	pthread_mutex_t iolock;
	_ecan_in in;
	_ecan_out out;
};

typedef struct _ecan_header {
	uint32_t sequence;   /**< seq num OR zero.
			If non-zero, the ELLSI-server discards CAN TX telegrams
			if the sequence number is less or equal to the sequence
			number of the last CAN TX telegram.

			For the other direction, the ELLSI-server will
			increment the sequence-element for every telegram send
			to the ELLSI-client (regardless if CAN RX data, a
			response to a previous command or a heartbeat).
			*/
	uint32_t command;    /**< one of ELLSI_CMD_* */
	uint32_t subcommand; /**< one of ELLSI_SUBCMD_* or ELLSI_IOCTL_* */
} _ecan_header;

static int _ecan_send_msg(ecan_connection_t *c,
		_ecan_header *eh, void *payload, size_t payload_bytes)
{
	struct iovec vec[2];
	struct ellsi_header head = {
		.magic         = htonl(ELLSI_MAGIC),
		.sequence      = htonl(eh->sequence),
		.command       = htonl(eh->command),
		.subcommand    = htonl(eh->subcommand),
		.payload_bytes = htonl(payload_bytes)
	};

	vec[0].iov_base = &head;
	vec[0].iov_len  = sizeof(head);

	vec[1].iov_base = payload;
	vec[1].iov_len  = payload_bytes;

	return writev(c->fd, vec, 2);
}

int  ecan_canid_add_range(ecan_connection_t *c,
		uint32_t can_id_start, uint32_t can_id_end)
{
	_ecan_header e = {
		.sequence   = 0,
		.command    = ELLSI_CMD_CTRL,
		.subcommand = ELLSI_IOCTL_CAN_ID_ADD
	};

	struct ellsi_can_id_range payload = {
		.start = htonl(can_id_start),
		.end   = htonl(can_id_end)
	};

	return _ecan_send_msg(c, &e, &payload, sizeof(payload));
}

ecan_connection_t *ecan_connect_to_fd(int udp_sock_fd)
{
	int r;
	ecan_connection_t *e = malloc(sizeof(e));
	if (!e)
		goto err_con_alloc;

	r = _ecan_in_init(&e->in);
	if (r)
		goto err_ein;
	r = _ecan_out_init(&e->out);
	if (r)
		goto err_eout;

err_eout:
	_ecan_in_destroy(&e->in);
err_ein:
	free(e);
err_con_alloc:
	return NULL;

}

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
ecan_connection_t *ecan_connect(char *host, uint16_t port)
{
	char service[6];
	sprintf(service, "%u", port);
	struct addrinfo hints = {
		.ai_family = AF_INET,
		.ai_socktype = SOCK_DGRAM,
		.ai_flags = AI_NUMERICSERV | AI_ADDRCONFIG
	};

	struct addrinfo *res;

	int ret = getaddrinfo(host, service, &hints, &res);
	if (ret) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(ret));
		return NULL;
	}

	int fd = socket(res->ai_family, res->ai_socktype, 0);
	if (fd < 0)
		return NULL;

	ret = connect(fd, res->ai_addr, res->ai_addrlen);
	if (ret < 0)
		return NULL;

	freeaddrinfo(res);

	return ecan_connect_to_fd(fd);
}

void ecan_disconnect(ecan_connection_t *e)
{
	_ecan_out_destroy(&e->out);
	_ecan_in_destroy(&e->in);
	free(e);
}


