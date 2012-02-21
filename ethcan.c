#include <pthread.h>
#include <stdlib.h>
#include "ethcan.h"

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

void ecan_disconnect(ecan_connection_t *e)
{
	_ecan_out_destroy(&e->out);
	_ecan_in_destroy(&e->in);
	free(e);
}


