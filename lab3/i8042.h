#ifndef _I8042_H
#define _I8042_H

#define KBD_IRQ					1
#define WAIT_KBC				20000
#define ESC_KEY					0x81
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

#endif
