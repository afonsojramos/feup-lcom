#ifndef _I8042_H
#define _I8042_H

#define KBD_IRQ					1
#define WAIT_KBC				20000
#define IN_BUF                  0x64
#define OUT_BUF					0x60
#define STAT_REG				0x64
#define KBC_CMD_REG     		0x64
#define PAR_ERR					BIT(7)
#define TO_ERR					BIT(6)
#define OBF						BIT(0)
#define BIT(n)    				(0x01<<(n))
#define SET_KBD_LEDS			0xED
#define IBF 					0x02
#define KBD_STATUS_PAR			0x80
#define KBD_STATUS_TIMEOUT		0x40
#define KBD_BUFF_ACK			0xFA
#define TIMEOUT					5

#define MOUSE_IRQ 				12
#define STATUS_REG              0x64
#define OUT_BUF					0x60
#define IN_BUF					0x64
#define MOUSE_ENABLE			0xA8
#define MOUSE_ENABLE_DATA		0xF4
#define MOUSE_DISABLE_DATA		0xF5
#define SET_STREAM_MODE 		0xEA
#define STATUS_REQUEST			0xE9
#define OBF						BIT(0)
#define BYTE_TO_MOUSE			0xD4
#define DEFAULT_WAIT			20000
#define BIT(n) 					(0x01<<(n))


/* KEYBOARD KEYS */
#define ESC_KEY					0x01
#define ESC_KEY_REL				0x81
#define ENTER_KEY				0x1C
#define W_KEY					0x11
#define W_KEY_REL				0x91
#define S_KEY					0x1F
#define S_KEY_REL				0x9F
#define A_KEY 					0x1E
#define A_KEY_REL				0x9E
#define D_KEY 					0x20
#define D_KEY_REL				0xA0

/* MOUSE BUTTONS */
#define LEFT_MOUSE				0x09
#define RIGHT_MOUSE				0x0A
#define LEFT_BUTTON 			BIT(0)
#define RIGHT_BUTTON 			BIT(1)
#define MIDDLE_BUTTON 			BIT(2)
#define X_SIGN 					BIT(4)
#define Y_SIGN 					BIT(5)
#define X_OVERFLOW 				BIT(6)
#define Y_OVERFLOW 				BIT(7)

#define ACK						0xFA

#endif
