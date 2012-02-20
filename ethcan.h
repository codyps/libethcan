#ifndef ETHCAN_H_
#define ETHCAN_H_

/*! @file ethcan.h
 *
 *  Interface for interacting with the Ethercan/2.
 */

typedef struct ecan_connection_t ecan_connection_t;

/*! defgroup ECAN instance setup
 * @{
 */

/* wrapper for ecan_create which also creates the socket.
 * @hostname: can also be an ip address (as a string).
 * @port:     if zero, default is used. */
ecan_connection_t *ecan_connect(char *hostname, uint16_t port);
ecan_connection_t *ecan_connect_to_fd(int udp_sock_fd);
void               ecan_disconnect(ecan_connection_t *ei);

/*! @defgroup CAN ID managment.
 * @{*/

/* range is inclusive */
int  ecan_canid_add_range(ecan_connection_t *c,
		uint32_t can_id_start, uint32_t can_id_end);
//int  ecan_canid_remove(ecan_connection_t *c, uint32_t can_id);
//bool ecan_canid_is_added(ecan_connection_t *c, uint32_t can_id);

/* wrapper for ecan_canid_add_range */
static inline int  ecan_canid_add(ecan_connection_t *c, uint32_t can_id)
{
	return ecan_canid_add_range(ei, can_id, can_id);
}

/*!@}*/

/*! @defgroup Sending and Recieving CAN datagrams.
 * @{*/
int ecan_send_to_can(ecan_connection_t *c, uint32_t can_id, void *in_buf, size_t bytes);
ssize_t ecan_recv_from_can(ecan_connection_t *c, void *out_buf, size_t bytes);
/*!@}*/

void ecan_flush(ecan_con

#endif
