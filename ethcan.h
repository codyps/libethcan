#ifndef ETHCAN_H_
#define ETHCAN_H_

/*! @file ethcan.h
 *
 *  Interface for interacting with the Ethercan/2.
 */

typedef struct ecan_instance;

/*! defgroup ECAN instance setup
 * @{
 */

/* wrapper for ecan_create which also creates the socket.
 * @hostname: can also be an ip address (as a string).
 * @port:     if zero, default is used. */
ecan_instance *ecan_open(char *hostname, uint16_t port);

ecan_instance *ecan_create(int udp_sock_fd);
void           ecan_destroy(ecan_instance *ei);

/*! @defgroup CAN ID managment.
 * @{*/

/* range is inclusive */
int  ecan_canid_add_range(ecan_instance *ei, uint32_t can_id_start, uint32_t can_id_end);
//int  ecan_canid_remove(ecan_instance *ei, uint32_t can_id);
//bool ecan_canid_is_added(ecan_instance *ei, uint32_t can_id);

/* wrapper for ecan_canid_add_range */
static inline int  ecan_canid_add(ecan_instance *ei, uint32_t can_id) {
	return ecan_canid_add_range(ei, can_id, can_id);
}

/*!@}*/

/*! @defgroup Sending and Recieving CAN datagrams.
 * @{*/
int ecan_send_to_can(ecan_instance *ei, uint32_t can_id, void *in_buf, size_t bytes);
ssize_t ecan_recv_from_can(ecan_instance *ei, void *out_buf, size_t bytes);
/*!@}*/

#endif
