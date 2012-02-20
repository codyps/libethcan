#ifndef LIBETHCAN_ELLSI_H_
#define LIBETHCAN_ELLSI_H_

#include <stdint.h>

#ifdef __CHECKER__
# define __bitwise__ __attribute__((bitwise))
#else
# define __bitwise__
#endif

#ifdef __CHECK_ENDIAN__
# define __bitwise __bitwise__
#else
# define __bitwise
#endif

/* network byte order for all items. */

struct ellsi_header {
	uint32_t magic;
#define ELLSI_MAGIC 0x454c5349

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
	uint32_t payload_bytes;
	uint32_t subcommand; /**< one of ELLSI_SUBCMD_* or ELLSI_IOCTL_* */
	uint8_t  reserved[32];
} __attribute__((packed,align));

#define ELLSI_CMD_NOP          0
#define ELLSI_CMD_CAN_TELEGRAM 1
#define ELLSI_CMD_HEARTBEAT    2
#define ELLSI_CMD_CTRL         3
#define ELLSI_CMD_REGISTER     4
#define ELLSI_CMD_REGISTERX    5

#define ELLSI_IOCTL_NOP               0
#define ELLSI_SUBCMD_NONE             0
#define ELLSI_IOCTL_CAN_ID_ADD        1
#define ELLSI_IOCTL_CAN_ID_DELETE     2
#define ELLSI_IOCTL_CAN_SET_BAUDRATE  3
#define ELLSI_IOCTL_CAN_GET_BAUDRATE  4
#define ELLSI_IOCTL_GET_LAST_STATE    5
#define ELLSI_SUBCMD_TXDONE           128
#define ELLSI_SUBCMD_AUTOACK          256

/**
 * with command = ELLSI_CMD_REGISTERX
 */
typedef ellsi_register_x {
	uint32_t heartbeat_interval;
	/**<
	  ELLSI server heartbeat interval in ms.  Use 0 for default value
	  (default is 2500 ms).  The valid range is 250 <= x <= 30000.
	  */
	uint32_t client_dead_multiplier;
	/**<
	  Number of missed heartbeat intervals / 10 before client is assumed to
	  be dead.
	  Valid = 10 to 100.
	  Default = 30 (Client assumed dead after missed intervals).
	  Use 0 for default.
	*/
	uint32_t can_tx_queue_sz; /**< default = 512. valid = 1 to 2048 */
	uint32_t can_rx_queue_sz; /**< default = 512. valid = 1 to 2048 */
	uint32_t max_can_in_udp;
	/**< maximum number of CAN grams to pack into a UDP gram.
	  default = CAN_READ_MAX_LEN = 50. valid = 1 to CAN_READ_MAXLEN */
	uint32_t send_accum_interval;
	/**< milliseconds to accumulate data for
	  prior to sending to client.  default = 0. Noted as not implimented in
	  manual version 1.5 (2010-02-22). */
	uint32_t reserved[8];
};


/* with command = ELLSI_CMD_CAN_TELEGRAM
 *
 * payload = 1 or more */
struct ellsi_can_gram {
	uint32_t id;
	uint8_t  len;          /**< not actually a 'len' */
	uint8_t  msg_lost;     /**< Counts lost CAN RX frames. Saturates at 255. */
	uint8_t  reserved[2];  /**< Not quite reserved */
	uint8_t  data[8];      /**< CAN data bytes */
	uint32_t timestamp[2]; /**< noted as not implimented in manual version 1.5 (2010-02-22) */
};

/* with command = ELLSI_CMD_CTRL
 * and    subcommand = ELLSI_IOCTL_CAN_ID_ADD
 *     or subcommand = ELLSI_IOCTL_CAN_ID_DELETE
 *
 * payload = 1 or more.
 */
struct ellsi_can_id_range {
	uint32_t start;
	uint32_t end;
};

/* with   subcommand = ELLSI_IOCTL_CAN_SET_BAUDRATE
 *     or subcommand = ELLSI_IOCTL_CAN_GET_BAUDRATE
 *
 * payload = 1 32bit uint.
 */
#define ELLSI_CAN_BIT_RATE_1000 0

/*
0x1 666.6
0x2 500
0x3 333.3
0x4 250
0x5 166
0x6 125
0x7 100
0x8 66.6
0x9 50
0xA 33.3
0xB 20
0xC 12.5
0xD 10
*/

/*
 * If the LSB (bit 31) of parameter baudrate is set to '1', the value will be evaluated differently. In this
 * case, the register value for the bit-timing registers BTR0 and BTR1 transmitted in modules with
 * CAN controllers 82C200, SJA1000, 82527 (and all other controllers with this baud rate structure) is
 * defined directly. For further information on this topic, see our esd NTCAN API documentatio
 */

/* with   subcmd = ELLSI_IOCTL_SET_SJA1000_ACMR
 *
 * XXX: not dealing with this */

/* subcmd = ELLSI_IOCTL_GET_LAST_STATE
 *
 * Sends the "lastState" variable back.
 * "lastState" contains the value of the last command processed by the ELLSI-server.
 *
 * Using the ELLSI_SUBCMD_AUTOACK, the ELLSI-server automatically dispatches this message.
 */
struct ellsi_last_state {
	uint32_t last_cmd;
	uint32_t last_sub_cmd;
	int32_t  last_state;
	uint32_t last_rx_seg;
	uint32_t reserved[4];
};

#define ELLSI_UDP_PORT_DEFAULT 2209

#endif
