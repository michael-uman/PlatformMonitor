/*
 * commands.h
 *
 *  Created on: Jun 29, 2019
 *      Author: muman
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_


/**
 * Commands used by the send thread...
 */
typedef enum {
	SENDCMD_START = 0x0080,
	SENDCMD_MSG = SENDCMD_START,
	SENDCMD_STATUS,
	SENDCMD_VERSION,
	SENDCMD_END,
} eSENDCMD;

/**
 * Commands used by the receive thread...
 */
typedef enum {
	RECVCMD_START = 0x0080,
	RECVCMD_HELLO = RECVCMD_START,	// Say Hello (not implemented)
	RECVCMD_LEDON,					// Turn on LED's
	RECVCMD_LEDOFF,					// Turn off LED's
	RECVCMD_VERSION,				// Send back the FW version
	RECVCMD_SETRTC,					// Set the RTC
	RECVCMD_END,
} eRECVCMD;

// Send message queue object
typedef struct {
	eSENDCMD 	cmd;
	uint32_t 	count;
	uint32_t 	buttonState;
} SENDMSGQUEUE_OBJ_t;

typedef struct {
	uint32_t	cmd;
	uint32_t	data;
} RECVCMD_t;

// Receive message queue object
typedef struct {
	RECVCMD_t	packet;
	uint32_t	count;
} RECVMSGQUEUE_OBJ_t;


#define LED_ONE		(1 << 0)
#define LED_TWO		(1 << 1)
#define LED_THREE	(1 << 2)
#define LED_FOUR	(1 << 3)

#define IS_LED_SET(v, m) ((v & m) != 0)

//
//// Turn the LED on
//#define CMD_LED1_ON		"11111111"
//// Turn the LED off
//#define CMD_LED1_OFF	"00000000"
//// Reset the system
//#define CMD_SYS_RST		"99999999"
//
#endif /* COMMANDS_H_ */
